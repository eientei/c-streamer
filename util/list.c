//
// Created by user on 2/17/17.
//

#include <stdlib.h>
#include "list.h"

void video_list_init(video_list_t *list) {
    list->capacity = 1;
    list->size = 0;
    list->nodes = calloc(1, sizeof(video_list_entry_t));
}

void video_list_deinit(video_list_t *list) {
    for (size_t i = 0; i < list->size; i++) {
        if (list->nodes[i].cb) {
            list->nodes[i].cb(list->nodes[i].data);
        }
    }

    list->capacity = 0;
    list->size = 0;
    free(list->nodes);
}

void video_list_add(video_list_t *list, void *data, free_cb cb) {
    if (list->size == list->capacity) {
        list->capacity <<= 1;
        list->nodes = realloc(list->nodes, list->capacity * sizeof(video_list_entry_t));
    }

    list->nodes[list->size].data = data;
    list->nodes[list->size].cb = cb;
    list->size++;
}

void video_list_remove(video_list_t *list, void *data) {
    for (size_t i = 0; i < list->size; i++) {
        if (list->nodes[i].data == data) {
            if (list->nodes[i].cb) {
                list->nodes[i].cb(list->nodes[i].data);
            }
            list->size--;
            list->nodes[i] = list->nodes[list->size];
            return;
        }
    }
}