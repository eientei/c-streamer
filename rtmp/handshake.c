//
// Created by user on 2/20/17.
//

#include <string.h>
#include <openssl/hmac.h>
#include "rtmp/rtmp.h"

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

static char SERVER_KEY_TEXT[] = {
        'G', 'e', 'n', 'u', 'i', 'n', 'e', ' ', 'A', 'd', 'o', 'b', 'e', ' ',
        'F', 'l', 'a', 's', 'h', ' ', 'M', 'e', 'd', 'i', 'a', ' ',
        'S', 'e', 'r', 'v', 'e', 'r', ' ',
        '0', '0', '1'
};
static char CLIENT_KEY_TEXT[] = {
        'G', 'e', 'n', 'u', 'i', 'n', 'e', ' ', 'A', 'd', 'o', 'b', 'e', ' ',
        'F', 'l', 'a', 's', 'h', ' ', 'P', 'l', 'a', 'y', 'e', 'r', ' ',
        '0', '0', '1'
};

static char SERVER_VERSION[] = {
        (char) 0x0D, (char) 0x0E, (char) 0x0A, (char) 0x0D
};

static size_t video_rtmp_handshake_findoffset(const char *data, size_t base) {
    size_t offset = 0;
    for (size_t i = 0; i < 4; i++) {
        offset += data[base+i] & 0xFF;
    }
    return (offset % 728) + base + 4;
}

static void video_rtmp_handshake_makedigest(const char *data, size_t datalen, size_t offset, const char *key, size_t keylen, char *digest) {
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, key, (int) keylen, EVP_sha256(), 0);
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

static int video_rtmp_handshake_finddigest(const char *data, size_t datalen, char *digest, size_t base, const char *key, size_t keylen) {
    size_t offset = video_rtmp_handshake_findoffset(data, base);
    video_rtmp_handshake_makedigest(data, datalen, offset, key, keylen, digest);
    return memcmp(digest, data+offset, 32) == 0;
}

static void video_rtmp_handshake_common(video_rtmp_local_t *local) {
    uv_buf_t buf = uv_buf_init(video_rtmp_alloc(local, 2048), 1537);
    buf.base[0] = 0x03;

    memset(&buf.base[1], 0, 4);
    memmove(&buf.base[5], SERVER_VERSION, 4);
    srand((unsigned int) time(NULL));
    for (size_t i = 9; i < 1537; i+=4) {
        int rnd = rand();
        memmove(&buf.base[i], &rnd, 4);
    }

    size_t offset = video_rtmp_handshake_findoffset(buf.base+1, 8);
    video_rtmp_handshake_makedigest(buf.base+1, 1536, offset, SERVER_KEY_TEXT, sizeof(SERVER_KEY_TEXT), buf.base+1+offset);
    video_rtmp_write(local, &buf);
}

static void video_rtmp_handshake_new(video_rtmp_local_t *local, char *digest) {
    video_rtmp_handshake_makedigest(digest, 32, 32, SERVER_KEY, sizeof(SERVER_KEY), digest);
    video_rtmp_handshake_makedigest(local->handshakebuf.base, 1536, 1504, digest, 32, local->handshakebuf.base+1504);
    video_rtmp_write(local, &local->handshakebuf);
}

static void video_rtmp_handshake_old(video_rtmp_local_t *local) {
    video_rtmp_write(local, &local->handshakebuf);
}

char *video_rtmp_handshake_s0(video_rtmp_local_t *local, char *ptr, size_t len) {
    if (*ptr++ != 0x03) {
        video_rtmp_disconnect(local);
        return ptr;
    }
    local->state = VIDEO_RTMP_STATE_S1;
    return ptr;
}

char *video_rtmp_handshake_s1(video_rtmp_local_t *local, char *ptr, size_t len) {
    size_t remaining = 1536 - local->handshakebuf.len;
    if (remaining > len) {
        remaining = len;
    }

    memmove(local->handshakebuf.base + local->handshakebuf.len, ptr, remaining);
    ptr += remaining;
    local->handshakebuf.len += remaining;

    if (local->handshakebuf.len != 1536) {
        return ptr;
    }

    char digest[32];
    int found = video_rtmp_handshake_finddigest(local->handshakebuf.base, 1536, digest, 772, CLIENT_KEY_TEXT, sizeof(CLIENT_KEY_TEXT)) ||
                video_rtmp_handshake_finddigest(local->handshakebuf.base, 1536, digest, 8, CLIENT_KEY_TEXT, sizeof(CLIENT_KEY_TEXT));

    video_rtmp_handshake_common(local);
    if (found) {
        video_rtmp_handshake_new(local, digest);
    } else {
        video_rtmp_handshake_old(local);
    }

    local->state = VIDEO_RTMP_STATE_S2;
    local->handshakebuf.len = 0;
    return ptr;
}

char *video_rtmp_handshake_s2(video_rtmp_local_t *local, char *ptr, size_t len) {
    size_t remaining = 1536 - local->handshakebuf.len;
    if (remaining > len) {
        remaining = len;
    }

    local->handshakebuf.len += remaining;
    ptr += remaining;

    if (local->handshakebuf.len != 1536) {
        return ptr;
    }

    local->state = VIDEO_RTMP_STATE_HEADER;
    local->msgheaderbuf.len = 0;
    return ptr;
}