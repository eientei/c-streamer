//
// Created by user on 2/12/17.
//

#include <stdlib.h>
#include "rtmp.h"
#include "buffer.h"



static void rtmp_alloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    *buf = uv_buf_init(bufslab_acquire(handle->loop->data, (int) suggested_size), (unsigned int) suggested_size);
}

static void rtmp_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    uv_close((uv_handle_t *) stream, (uv_close_cb) free);
    bufslab_unref(buf->base);
}

void rtmp_init() {

}

void rtmp_connected(uv_stream_t* stream) {
    uv_read_start(stream, rtmp_alloc, rtmp_read);
}