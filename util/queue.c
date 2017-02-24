//
// Created by user on 2/18/17.
//

#include <stdlib.h>
#include "queue.h"

void video_queue_init(video_queue_t *queue) {
    queue->head.data = 0;
    queue->head.next = 0;
    uv_mutex_init(&queue->mutex);
}

void video_queue_deinit(video_queue_t *queue) {
    for (video_queue_node_t *node = queue->head.next; node;) {
        video_queue_node_t *tmpnode = node->next;
        if (node->cb) {
            node->cb(node->data);
        }
        free(node);
        node = tmpnode;
    }
    uv_mutex_destroy(&queue->mutex);
}

void video_queue_enqueue(video_queue_t *queue, void *data, free_cb cb) {
    uv_mutex_lock(&queue->mutex);
    video_queue_node_t *node = &queue->head;
    while (node->next) {
        node = node->next;
    }
    node->next = calloc(1, sizeof(video_queue_node_t));
    node->next->data = data;
    node->next->next = 0;
    node->next->cb = cb;
    uv_mutex_unlock(&queue->mutex);
}

void *video_queue_dequeue(video_queue_t *queue) {
    uv_mutex_lock(&queue->mutex);
    if (!queue->head.next) {
        uv_mutex_unlock(&queue->mutex);
        return 0;
    }
    void *data = queue->head.next->data;
    video_queue_node_t *next = queue->head.next->next;
    free(queue->head.next);
    queue->head.next = next;
    uv_mutex_unlock(&queue->mutex);
    return data;
}
