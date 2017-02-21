//
// Created by user on 2/18/17.
//

#include <stdlib.h>
#include "rtmp/rtmp.h"
#include "mp4/mp4.h"

static void video_async_wrapper(uv_async_t *async) {
    video_thread_t *thread = async->loop->data;
    video_async_t *video = video_queue_dequeue(&thread->queue);
    int close = 0;
    while (video) {
        thread->async(thread, video);
        if (video->cb) {
            video->cb(video->data);
        }
        if (video->type == VIDEO_ASYNC_TYPE_STOP) {
            close = 1;
        }
        free(video);
        video = video_queue_dequeue(&thread->queue);
    }

    if (close) {
        uv_close((uv_handle_t *) async, 0);
    }
}

static void video_global_entry(void *arg) {
    video_thread_t *thread = arg;
    uv_loop_init(&thread->loop);
    thread->loop.data = thread;

    uv_async_init(&thread->loop, &thread->latch, video_async_wrapper);

    thread->entry(thread);

    printf("%s loop ready\n", thread->name);
    uv_run(&thread->loop, UV_RUN_DEFAULT);
    uv_mutex_lock(&thread->global->mutex);
    printf("%s loop done\n", thread->name);
    thread->exit(thread);

    while (uv_loop_close(&thread->loop) != 0) {
        uv_run(&thread->loop, UV_RUN_ONCE);
    }

    video_slab_stats(&thread->slab, thread->name);
    uv_mutex_unlock(&thread->global->mutex);
}

void video_make_async(uv_async_t *async, video_async_type type, void *data, free_cb freedata) {
    video_async_t *video_async = calloc(1, sizeof(video_async_t));
    video_async->type = type;
    video_async->data = data;
    video_async->cb = freedata;
    video_thread_t *thread = async->loop->data;
    video_queue_enqueue(&thread->queue, video_async);
    uv_async_send(async);
}

void video_global_init(video_global_t *global) {
    uv_mutex_init(&global->mutex);

    global->main.async = video_main_async;
    video_queue_init(&global->main.queue);
    global->main.global = global;
    global->main.latch.data = &global->main.queue;
    global->main.name = "Main";
    uv_default_loop()->data = &global->main;
    uv_async_init(uv_default_loop(), &global->main.latch, video_async_wrapper);

    video_slab_init(&global->rtmp.slab, 16, VIDEO_RTMP_MESSAGE_MAX, 0);
    global->rtmp.entry = video_rtmp_entry;
    global->rtmp.exit = video_rtmp_exit;
    global->rtmp.async = video_rtmp_async;
    global->rtmp.global = global;
    video_queue_init(&global->rtmp.queue);
    global->rtmp.latch.data = &global->rtmp.queue;
    global->rtmp.name = "RTMP";
    uv_thread_create(&global->rtmp.thread, video_global_entry, &global->rtmp);

    video_slab_init(&global->mp4.slab, 16, 65536, 0);
    global->mp4.entry = video_mp4_entry;
    global->mp4.exit = video_mp4_exit;
    global->mp4.async = video_mp4_async;
    global->mp4.global = global;
    video_queue_init(&global->mp4.queue);
    global->mp4.latch.data = &global->mp4.queue;
    global->mp4.name = "MP4";
    uv_thread_create(&global->mp4.thread, video_global_entry, &global->mp4);
}

void video_global_deinit(video_global_t *global) {
    uv_thread_join(&global->rtmp.thread);
    uv_thread_join(&global->mp4.thread);

    video_queue_deinit(&global->main.queue);
    video_queue_deinit(&global->rtmp.queue);
    video_queue_deinit(&global->mp4.queue);

    video_slab_deinit(&global->rtmp.slab);
    video_slab_deinit(&global->mp4.slab);

    uv_mutex_destroy(&global->mutex);
}