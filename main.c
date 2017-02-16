#include <uv.h>
#include <stdlib.h>
#include "rtmp/rtmp.h"
#include "util/buffer.h"

static uv_async_t *main_latch;
static uv_async_t *rtmp_latch;
static int stop = 0;

uv_loop_t* create_loop() {
    uv_loop_t *loop = malloc(sizeof(uv_loop_t));
    if (loop) {
        uv_loop_init(loop);
    }
    return loop;
}

void free_walker(uv_handle_t *handle, void *arg) {
    uv_close(handle, arg);
}

void stop_loop(uv_loop_t *loop, void *arg) {
    uv_stop(loop);
    uv_walk(loop, free_walker, arg);
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
    rtmp_init();
    uv_loop_t *loop = create_loop();
    loop->data = malloc(sizeof(bufslab_t));
    bufslab_init(loop->data, 128, 16777216, 1);

    uv_tcp_t *server = malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, server);

    struct sockaddr_in addr;
    uv_ip4_addr("0.0.0.0", 1935, &addr);
    uv_tcp_bind(server, (const struct sockaddr *) &addr, 0);
    int r = uv_listen((uv_stream_t *) server, 16, on_new_rtmp_connection);
    if (r) {
        fprintf(stderr, "Listen error: %s\n", uv_strerror(r));
        stop = 1;
    }

    uv_async_init(loop, rtmp_latch, 0);

    while (!stop) {
        uv_run(loop, UV_RUN_ONCE);
    }

    rtmp_deinit();
    bufslab_print(loop->data, "RTMP slab usage stats:\n");
    bufslab_deinit(loop->data);
    stop_loop(loop, free);
    free(loop->data);
    free(loop);
    rtmp_latch = 0;
}

void sigint_handler(uv_signal_t *handler, int signum) {
    stop = 1;
}

int main(int argc, char **argv) {
    main_latch = calloc(1, sizeof(uv_async_t));
    rtmp_latch = calloc(1, sizeof(uv_async_t));

    uv_signal_t sigint;
    uv_signal_init(uv_default_loop(), &sigint);
    uv_signal_start(&sigint, sigint_handler, SIGINT);

    uv_thread_t rtmp_thread;
    uv_thread_create(&rtmp_thread, rtmp_entry, 0);

    uv_async_init(uv_default_loop(), main_latch, 0);

    while (!stop) {
        uv_run(uv_default_loop(), UV_RUN_ONCE);
    }

    if (rtmp_latch) {
        uv_async_send(rtmp_latch);
    }

    stop_loop(uv_default_loop(), 0);
    uv_thread_join(&rtmp_thread);
    free(main_latch);

    fprintf(stderr, "We are not working any more, shoo!\n");
    return 0;
}