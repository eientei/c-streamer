#include <uv.h>
#include <stdlib.h>
#include "rtmp.h"
#include "buffer.h"

static uv_async_t rtmp_latch;
static int stop = 0;

uv_loop_t* create_loop() {
    uv_loop_t *loop = malloc(sizeof(uv_loop_t));
    if (loop) {
        uv_loop_init(loop);
    }
    return loop;
}

void free_walker(uv_handle_t *handle, void *arg) {
    uv_close(handle, 0);
}

void stop_loop(uv_loop_t *loop) {
    uv_stop(loop);
    uv_walk(loop, free_walker, 0);
    do {
        uv_run(loop, UV_RUN_ONCE);
    } while (uv_loop_close(loop) != 0);
}

void on_new_rtmp_connection(uv_stream_t *stream, int status) {
    if (status < 0) {
        fprintf(stderr, "New RTMP connetion error %s\n", uv_strerror(status));
        return;
    }

    uv_tcp_t *client = malloc(sizeof(uv_tcp_t));
    uv_tcp_init(stream->loop, client);
    if (uv_accept(stream, (uv_stream_t *) client) == 0) {
        rtmp_connected((uv_stream_t *) client);
    } else {
        uv_close((uv_handle_t *) client, (uv_close_cb) free);
    }
}

void rtmp_entry(void *userp) {
    uv_loop_t *loop = create_loop();
    loop->data = malloc(sizeof(bufslab_t));
    bufslab_init(loop->data, 128, 16777216, 1);

    uv_tcp_t server;
    uv_tcp_init(loop, &server);

    struct sockaddr_in addr;
    uv_ip4_addr("0.0.0.0", 1935, &addr);
    uv_tcp_bind(&server, (const struct sockaddr *) &addr, 0);
    int r = uv_listen((uv_stream_t *) &server, 16, on_new_rtmp_connection);
    if (r) {
        fprintf(stderr, "Listen error: %s\n", uv_strerror(r));
        goto rtmp_entry_free;
    }

    uv_async_init(loop, &rtmp_latch, 0);

    while (!stop) {
        uv_run(loop, UV_RUN_ONCE);
    }

    rtmp_entry_free:
    bufslab_deinit(loop->data);
    stop_loop(loop);
    free(loop->data);
    free(loop);
}

void sigint_handler(uv_signal_t *handler, int signum) {
    stop = 1;
}

int main(int argc, char **argv) {
    uv_signal_t sigint;
    uv_signal_init(uv_default_loop(), &sigint);
    uv_signal_start(&sigint, sigint_handler, SIGINT);

    rtmp_init();
    uv_thread_t rtmp_thread;
    uv_thread_create(&rtmp_thread, rtmp_entry, 0);

    while (!stop) {
        uv_run(uv_default_loop(), UV_RUN_ONCE);
    }

    stop_loop(uv_default_loop());
    uv_async_send(&rtmp_latch);
    uv_thread_join(&rtmp_thread);
    return 0;
}