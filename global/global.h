//
// Created by user on 2/17/17.
//

#ifndef VIDEO_GLOBAL_H
#define VIDEO_GLOBAL_H

#include <uv.h>
#include "util/buffer.h"
#include "util/queue.h"
#include "util/generic.h"

typedef enum video_async_type_e video_async_type;
enum video_async_type_e {
    VIDEO_ASYNC_TYPE_NOOP,
    VIDEO_ASYNC_TYPE_STOP,
    VIDEO_ASYNC_TYPE_STAT
};

typedef struct video_async_s video_async_t;
struct video_async_s {
    video_async_type type;
    void *data;
    free_cb cb;
};

typedef struct video_thread_s video_thread_t;

typedef void (*video_thread_cb)(video_thread_t *thread);
typedef void (*video_async_cb)(video_thread_t *thread, video_async_t *async);

struct video_thread_s {
    struct video_global_s *global;
    uv_thread_t thread;
    uv_loop_t loop;
    video_queue_t queue;
    uv_async_t latch;
    video_thread_cb entry;
    video_thread_cb exit;
    video_async_cb async;
    video_slab_t slab;

    void *data;
    char *name;
};

typedef struct video_global_s video_global_t;
struct video_global_s {
    video_thread_t main;
    video_thread_t rtmp;
    video_thread_t mp4;

    uv_mutex_t mutex;
    uv_signal_t sigint;
    uv_signal_t sigusr;
};

void video_make_async(uv_async_t *async, video_async_type type, void *data, free_cb freedata);

void video_global_init(video_global_t *global);
void video_global_deinit(video_global_t *global);

void video_main_async(video_thread_t *thread, video_async_t *async);

#endif //VIDEO_GLOBAL_H
