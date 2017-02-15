//
// Created by user on 2/12/17.
//

#include <stdlib.h>
#include <string.h>
#include <openssl/hmac.h>
#include "rtmp/rtmp.h"
#include "util/buffer.h"

static char SERVER_KEY[] = {
        'G', 'e', 'n', 'u', 'i', 'n', 'e', ' ', 'A', 'd', 'o', 'b', 'e', ' ',
        'F', 'l', 'a', 's', 'h', ' ', 'M', 'e', 'd', 'i', 'a', ' ',
        'S', 'e', 'r', 'v', 'e', 'r', ' ',
        '0', '0', '1',
        (char) 0xF0, (char) 0xEE, (char) 0xC2, (char) 0x4A, (char) 0x80, (char) 0x68,
        (char) 0xBE, (char) 0xE8, (char) 0x2E, (char) 0x00, (char) 0xD0, (char) 0xD1,
        (char) 0x02, (char) 0x9E, (char) 0x7E, (char) 0x57, (char) 0x6E, (char) 0xEC,
        (char) 0x5D, (char) 0x2D, (char) 0x29, (char) 0x80, (char) 0x6F, (char) 0xAB,
        (char) 0x93, (char) 0xB8, (char) 0xE6, (char) 0x36, (char) 0xCF, (char) 0xEB,
        (char) 0x31, (char) 0xAE

};

static char CLIENT_KEY[] = {
        'G', 'e', 'n', 'u', 'i', 'n', 'e', ' ', 'A', 'd', 'o', 'b', 'e', ' ',
        'F', 'l', 'a', 's', 'h', ' ', 'P', 'l', 'a', 'y', 'e', 'r', ' ',
        '0', '0', '1',
        (char) 0xF0, (char) 0xEE, (char) 0xC2, (char) 0x4A, (char) 0x80, (char) 0x68,
        (char) 0xBE, (char) 0xE8, (char) 0x2E, (char) 0x00, (char) 0xD0, (char) 0xD1,
        (char) 0x02, (char) 0x9E, (char) 0x7E, (char) 0x57, (char) 0x6E, (char) 0xEC,
        (char) 0x5D, (char) 0x2D, (char) 0x29, (char) 0x80, (char) 0x6F, (char) 0xAB,
        (char) 0x93, (char) 0xB8, (char) 0xE6, (char) 0x36, (char) 0xCF, (char) 0xEB,
        (char) 0x31, (char) 0xAE
};

static char SERVER_KEY_TEXT[36];
static char CLIENT_KEY_TEXT[30];

static char SERVER_VERSION[] = {
        (char) 0x0D, (char) 0x0E, (char) 0x0A, (char) 0x0D
};

static rtmp_global_t global;

static uv_buf_t rtmp_alloc_buf(uv_handle_t *handle, size_t size) {
    return uv_buf_init(bufslab_acquire(handle->loop->data, (int) size), (unsigned int) size);
}

static void rtmp_read_alloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    suggested_size = 4096;
    *buf = uv_buf_init(bufslab_acquire(handle->loop->data, (int) suggested_size), (unsigned int) suggested_size);
}

static size_t rtmp_handshake_0(rtmp_context_t *context, size_t nread, const char *data) {
    if (data[0] != 0x03) {
        rtmp_disconnect(context->stream);
        return 0;
    }
    context->state = RTMP_STATE_S1;
    return 1;
}

static int rtmp_handshake_findoffset(const char *data, int base) {
    int offset = 0;
    for (int i = 0; i < 4; i++) {
        offset += data[base+i] & 0xFF;
    }
    return (offset % 728) + base + 4;
}

static void rtmp_handshake_makedigest(const char *data, int datalen, int offset, char *key, int keylen, char *digest) {
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, key, keylen, EVP_sha256(), 0);
    if (offset > 0) {
        HMAC_Update(&ctx, (const unsigned char *) data, (size_t) offset);
    }

    if (offset+32 < datalen) {
        HMAC_Update(&ctx, (const unsigned char *) (data + offset + 32), (size_t) (datalen - offset - 32));
    }

    unsigned int resultlen = 32;
    HMAC_Final(&ctx, (unsigned char *) digest, &resultlen);
    HMAC_CTX_cleanup(&ctx);
}

static int rtmp_handshake_finddigest(const char *data, int datalen, char *digest, int base, char *key, int keylen) {
    int offset = rtmp_handshake_findoffset(data, base);
    rtmp_handshake_makedigest(data, datalen, offset, key, keylen, digest);
    if (memcmp(digest, data+offset, 32) == 0) {
        return 1;
    }
    return 0;
}

static void rtmp_write(uv_write_t *req, int status) {
    bufslab_unref(req->data);
    free(req);
}

static void rtmp_handshake_common(rtmp_context_t *context) {
    uv_buf_t buf = rtmp_alloc_buf(context->stream, 1537);
    buf.base[0] = 0x03;

    memset(&buf.base[1], 0, 4);
    memmove(&buf.base[5], SERVER_VERSION, 4);
    srand((unsigned int) time(NULL));
    for (int i = 9; i < 1537; i+=4) {
        int rnd = rand();
        memmove(&buf.base[i], &rnd, 4);
    }

    int offset = rtmp_handshake_findoffset(buf.base+1, 8);
    rtmp_handshake_makedigest(buf.base+1, 1536, offset, SERVER_KEY_TEXT, sizeof(SERVER_KEY_TEXT), buf.base+1+offset);
    uv_write_t *req = malloc(sizeof(uv_write_t));
    req->data = buf.base;
    uv_write(req, (uv_stream_t *) context->stream, &buf, 1, rtmp_write);
}

static void rtmp_handshake_new(rtmp_context_t *context, char *digest) {
    rtmp_handshake_makedigest(digest, 32, 32, SERVER_KEY, sizeof(SERVER_KEY), digest);
    rtmp_handshake_makedigest(context->handshakebuf.base, 1536, 1504, digest, 32, context->handshakebuf.base+1504);

    uv_write_t *req = malloc(sizeof(uv_write_t));
    req->data = context->handshakebuf.base;
    uv_write(req, (uv_stream_t *) context->stream, &context->handshakebuf, 1, rtmp_write);
}

static size_t rtmp_handshake_1(rtmp_context_t *context, size_t nread, const char *data) {
    size_t tocopy = 1536 - context->handshakebuf.len;
    if (tocopy > nread) {
        tocopy = nread;
    }

    memmove(context->handshakebuf.base+context->handshakebuf.len, data, tocopy);
    context->handshakebuf.len += nread;

    if (context->handshakebuf.len != 1536) {
        return tocopy;
    }

    char digest[32];
    int found = rtmp_handshake_finddigest(context->handshakebuf.base, 1536, digest, 772, CLIENT_KEY_TEXT, sizeof(CLIENT_KEY_TEXT)) ||
                rtmp_handshake_finddigest(context->handshakebuf.base, 1536, digest, 8, CLIENT_KEY_TEXT, sizeof(CLIENT_KEY_TEXT));

    rtmp_handshake_common(context);
    if (found) {
        rtmp_handshake_new(context, digest);
    } else {

    }

    return tocopy;
}

static void rtmp_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    if (nread < 0) {
        rtmp_disconnect((uv_handle_t *) stream);
    } else {
        rtmp_context_t *context = stream->data;
        size_t processed = 0;
        while (processed < nread) {
            size_t oldprocessed = processed;
            switch (context->state) {
                case RTMP_STATE_S0:
                    processed += rtmp_handshake_0(context, (size_t) (nread - processed), buf->base + processed);
                    break;
                case RTMP_STATE_S1:
                    processed += rtmp_handshake_1(context, (size_t) (nread - processed), buf->base + processed);
                    break;
                case RTMP_STATE_S2:
                    break;
                case RTMP_STATE_HEADER:
                    break;
                case RTMP_STATE_BODY:
                    break;
            }
            if (oldprocessed == processed) {
                fprintf(stderr, "[%p] %d bytes not processed, being skipped\n", context, (int) (nread - processed));
                break;
            }
        }
    }
    bufslab_unref(buf->base);
}

static rtmp_context_t* rtmp_create_context(uv_stream_t *stream) {
    rtmp_context_t *context = malloc(sizeof(rtmp_context_t));
    arraylist_init(&context->chunks);
    context->inchunk = 128;
    context->outchunk = 128;
    context->kind = RTMP_KIND_UNKNOWN;
    context->state = RTMP_STATE_S0;
    context->stream = (uv_handle_t *) stream;
    context->handshakebuf = rtmp_alloc_buf((uv_handle_t *) stream, 1536);
    context->handshakebuf.len = 0;
    return context;
}

static void rtmp_free_context(void *data) {
    rtmp_context_t *context = data;
    arraylist_deinit(&context->chunks);
    bufslab_unref(context->handshakebuf.base);
    free(context);
}

void rtmp_init() {
    arraylist_init(&global.channels);
    arraylist_init(&global.contexts);

    memmove(SERVER_KEY_TEXT, SERVER_KEY, sizeof(SERVER_KEY_TEXT));
    memmove(CLIENT_KEY_TEXT, CLIENT_KEY, sizeof(CLIENT_KEY_TEXT));
}

void rtmp_connected(uv_stream_t* stream) {
    rtmp_context_t *context = rtmp_create_context(stream);
    printf("[%p] connected\n", context);
    arraylist_add(&global.contexts, context, rtmp_free_context);
    stream->data = context;
    uv_read_start(stream, rtmp_read_alloc, rtmp_read);
}

void rtmp_disconnect(uv_handle_t *stream) {
    uv_close(stream, rtmp_disconnected);
}

void rtmp_disconnected(uv_handle_t *stream) {
    printf("[%p] disconnected\n", stream->data);
    arraylist_rem(&global.contexts, stream->data);
    free(stream);
}

void rtmp_deinit() {
    arraylist_deinit(&global.channels);
    arraylist_deinit(&global.contexts);
}