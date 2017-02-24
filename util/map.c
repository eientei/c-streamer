//
// Created by user on 2/17/17.
//

#include <string.h>
#include <stdlib.h>
#include "map.h"

static void video_map_entry_free(void *ptr) {
    video_map_entry_t *entry = ptr;
    if (entry->cb) {
        entry->cb(entry->value);
    }
    free(entry->key);
    free(entry);
}

void video_map_init(video_map_t *map) {
    video_list_init(map);
}

void video_map_deinit(video_map_t *map) {
    video_list_deinit(map);
}

void video_map_put(video_map_t *map, char *key, void *value, free_cb cb) {
    video_map_entry_t *entry = calloc(1, sizeof(video_map_entry_t));
    entry->key = strdup(key);
    entry->value = value;
    entry->cb = cb;
    video_list_add(map, entry, video_map_entry_free);
}

void video_map_remove(video_map_t *map, char *key) {
    for (size_t i = 0; i < map->size; i++) {
        video_map_entry_t *entry = map->nodes[i].data;
        if (strcmp(key, entry->key) == 0) {
            video_list_remove(map, entry);
            return;
        }
    }
}

void *video_map_get(video_map_t *map, char *key) {
    for (size_t i = 0; i < map->size; i++) {
        video_map_entry_t *entry = map->nodes[i].data;
        if (strcmp(key, entry->key) == 0) {
            return entry->value;
        }
    }
    return 0;
}