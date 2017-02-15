//
// Created by user on 2/14/17.
//

#include <stdlib.h>
#include "list.h"

void arraylist_init(arraylist_t *list) {
    list->capacity = 1;
    list->size = 0;
    list->nodes = calloc(1, sizeof(arraynode_t));
}

void arraylist_deinit(arraylist_t *list) {
    for (int i = 0; i < list->size; i++) {
        if (list->nodes[i].cb) {
            list->nodes[i].cb(list->nodes[i].data);
        }
    }
    list->capacity = 0;
    list->size = 0;
    free(list->nodes);
}

void arraylist_add(arraylist_t *list, void *data, free_cb cb) {
    if (list->size == list->capacity) {
        list->capacity <<= 1;
        list->nodes = realloc(list->nodes, sizeof(list->nodes) * sizeof(arraynode_t));
    }
    list->nodes[list->size].cb = cb;
    list->nodes[list->size].data = data;

    list->size++;
}

void arraylist_rem(arraylist_t *list, void *data) {
    int idx;
    for (idx = 0; idx < list->size; idx++) {
        if (list->nodes[idx].data == data) {
            break;
        }
    }
    if (idx == list->size) {
        return;
    }

    if (list->nodes[idx].cb) {
        list->nodes[idx].cb(list->nodes[idx].data);
    }
    list->size--;
    list->nodes[idx] = list->nodes[list->size];
}

void *arraylist_get(arraylist_t *list, int idx) {
    return list->nodes[idx].data;
}

int arraylist_indexof(arraylist_t *list, void *data) {
    for (int i = 0; i < list->size; i++) {
        if (list->nodes[i].data == data) {
            return i;
        }
    }
    return -1;
}

void arraylist_walk(arraylist_t *list, walk_cb walker, void *data) {
    for (int i = 0; i < list->size; i++) {
        arrayaction action = walker(list->nodes[i].data, data);
        switch (action) {
            case ARRAY_ACTION_CONTINUE:
                break;
            case ARRAY_ACTION_DELETE:
                arraylist_rem(list, list->nodes[i].data);
                i--;
                break;
            case ARRAY_ACTION_STOP:
                i = list->size;
                break;
        }
    };
};