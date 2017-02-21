//
// Created by user on 2/17/17.
//

#ifndef VIDEO_LIST_H
#define VIDEO_LIST_H

#include "util/generic.h"

typedef struct video_list_node_s video_list_entry_t;
struct video_list_node_s {
    void *data;
    free_cb cb;
};

typedef struct video_list_s video_list_t;
struct video_list_s {
    size_t capacity;
    size_t size;
    video_list_entry_t *nodes;
};

void video_list_init(video_list_t *list);
void video_list_deinit(video_list_t *list);
void video_list_add(video_list_t *list, void *data, free_cb cb);
void video_list_remove(video_list_t *list, void *data);

#endif //VIDEO_LIST_H
