//
// Created by user on 2/18/17.
//

#ifndef VIDEO_QUEUE_H
#define VIDEO_QUEUE_H

#include <uv.h>
#include "util/generic.h"

typedef struct video_queue_node_s video_queue_node_t;
struct video_queue_node_s {
    video_queue_node_t *next;
    void *data;
    free_cb cb;
};

typedef struct video_queue_s video_queue_t;
struct video_queue_s {
    video_queue_node_t head;
    uv_mutex_t mutex;
};

void video_queue_init(video_queue_t *queue);
void video_queue_deinit(video_queue_t *queue);
void video_queue_enqueue(video_queue_t *queue, void *data, free_cb cb);
void *video_queue_dequeue(video_queue_t *queue);

#endif //VIDEO_QUEUE_H
