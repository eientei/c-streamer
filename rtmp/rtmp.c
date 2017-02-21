//
// Created by user on 2/18/17.
//

#include <stdlib.h>
#include "rtmp.h"

static void video_rtmp_local_init(video_rtmp_local_t *local);
static void video_rtmp_local_deinit(video_rtmp_local_t *local);
static void video_rtmp_allocbuf(uv_handle_t *handle, size_t size, uv_buf_t *buf);
static void video_rtmp_readconn(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);
static void video_rtmp_newconn(uv_stream_t *client, int status);
static void video_rtmp_remconn(uv_handle_t *handle);
static void video_rtmp_writedone(uv_write_t *req, int status);

static void video_rtmp_local_init(video_rtmp_local_t *local) {
    local->inchunksiz = 128;
    local->outchunksiz = 128;
    local->state = VIDEO_RTMP_STATE_S0;
    local->kind = VIDEO_RTMP_KIND_UNDECIDED;
    local->header = 0;
    video_list_init(&local->chunks);
    local->handshakebuf = uv_buf_init(video_rtmp_alloc(local, 2048), 0);
    local->msgheaderbuf = uv_buf_init(video_rtmp_alloc(local, 128), 0);
}

static void video_rtmp_local_deinit(video_rtmp_local_t *local) {
    video_list_deinit(&local->chunks);
    if (local->state == VIDEO_RTMP_STATE_S0 || local->state == VIDEO_RTMP_STATE_S1) {
        video_slab_unref(local->handshakebuf.base);
    }
    video_slab_unref(local->msgheaderbuf.base);
}

static void video_rtmp_allocbuf(uv_handle_t *handle, size_t size, uv_buf_t *buf) {
    video_thread_t *thread = handle->loop->data;
    *buf = uv_buf_init(video_slab_alloc(&thread->slab, 2048), 2048);
}

static void video_rtmp_readconn(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    video_rtmp_local_t *local = stream->data;
    if (nread < 0) {
        video_rtmp_disconnect(local);
    } else {
        char *ptr = buf->base;
        while (ptr - buf->base < nread && !uv_is_closing((const uv_handle_t *) stream)) {
            video_rtmp_state prevstate = local->state;
            char *prevptr = ptr;
            switch (local->state) {
                case VIDEO_RTMP_STATE_S0:
                    ptr = video_rtmp_handshake_s0(local, ptr, nread - (ptr - buf->base));
                    break;
                case VIDEO_RTMP_STATE_S1:
                    ptr = video_rtmp_handshake_s1(local, ptr, nread - (ptr - buf->base));
                    break;
                case VIDEO_RTMP_STATE_S2:
                    ptr = video_rtmp_handshake_s2(local, ptr, nread - (ptr - buf->base));
                    break;
                case VIDEO_RTMP_STATE_HEADER:
                    ptr = video_rtmp_header(local, ptr, nread - (ptr - buf->base));
                    break;
                case VIDEO_RTMP_STATE_BODY:
                    ptr = video_rtmp_body(local, ptr, nread - (ptr - buf->base));
                    break;
            }
            if (prevptr == ptr) {
                printf("[%p] %ld bytes not processed\n", local, nread - (ptr - buf->base));
                if (prevstate == local->state) {
                    video_rtmp_disconnect(local);
                    break;
                }
            }
        }
    }
    video_slab_unref(buf->base);
}

static void video_rtmp_newconn(uv_stream_t *stream, int status) {
    video_thread_t *thread = stream->loop->data;
    video_rtmp_global_t *global = thread->data;
    video_rtmp_local_t *local = video_slab_alloc(&thread->slab, sizeof(video_rtmp_local_t));

    uv_tcp_init(stream->loop, &local->client);
    uv_accept(stream, (uv_stream_t *) &local->client);

    video_rtmp_local_init(local);
    local->client.data = local;

    video_list_add(&global->clients, local, 0);

    printf("[%p] connected\n", local);
    uv_read_start((uv_stream_t *) &local->client, video_rtmp_allocbuf, video_rtmp_readconn);
}

static void video_rtmp_remconn(uv_handle_t *handle) {
    video_thread_t *thread = handle->loop->data;
    video_rtmp_global_t *global = thread->data;
    video_rtmp_local_t *local = handle->data;

    printf("[%p] disconnected\n", local);

    video_list_remove(&global->clients, local);
    video_rtmp_local_deinit(local);
    video_slab_unref(local);
}

static void video_rtmp_writedone(uv_write_t *req, int status) {
    video_slab_unref(req->data);
    video_slab_unref(req);
}

void video_rtmp_entry(video_thread_t *thread) {
    video_rtmp_global_t *global = calloc(1, sizeof(video_rtmp_global_t));
    video_list_init(&global->clients);

    thread->data = global;

    uv_tcp_init(&thread->loop, &global->listener);

    struct sockaddr_in addr;
    uv_ip4_addr("0.0.0.0", 1935, &addr);
    uv_tcp_bind(&global->listener, (const struct sockaddr *) &addr, 0);
    int r = uv_listen((uv_stream_t *) &global->listener, 16, video_rtmp_newconn);

    if (r) {
        printf("RTMP Listen error: %s\n", uv_strerror(r));
    }
}

void video_rtmp_async(video_thread_t *thread, video_async_t *async) {
    video_rtmp_global_t *global = thread->data;
    switch (async->type) {
        default:
        case VIDEO_ASYNC_TYPE_NOOP:
            break;
        case VIDEO_ASYNC_TYPE_STOP:
            for (size_t i = 0; i < global->clients.size; i++) {
                video_rtmp_local_t *local = global->clients.nodes[i].data;
                video_rtmp_disconnect(local);
            }
            video_list_deinit(&global->clients);
            uv_close((uv_handle_t *) &global->listener, 0);
            break;
        case VIDEO_ASYNC_TYPE_STAT:
            video_slab_stats(&thread->slab, thread->name);
            break;
    }
}

void video_rtmp_exit(video_thread_t *thread) {
    video_rtmp_global_t *global = thread->data;
    free(global);
}

void video_rtmp_disconnect(video_rtmp_local_t *local) {
    if (!uv_is_closing((const uv_handle_t *) &local->client)) {
        uv_close((uv_handle_t *) &local->client, video_rtmp_remconn);
    }
}

void *video_rtmp_alloc(video_rtmp_local_t *local, size_t size) {
    video_thread_t *thread = local->client.loop->data;
    return video_slab_alloc(&thread->slab, size);
}

void video_rtmp_write(video_rtmp_local_t *local, uv_buf_t *buf) {
    uv_write_t *req = video_rtmp_alloc(local, sizeof(uv_write_t));
    req->data = buf->base;
    uv_write(req, (uv_stream_t *) &local->client, buf, 1, video_rtmp_writedone);
}