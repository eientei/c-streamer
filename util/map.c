//
// Created by user on 2/16/17.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "map.h"

static void amap_free_entry(void *ptr) {
    amap_entry_t *entry = ptr;
    if (entry->cb) {
        entry->cb(entry->value);
    }
    free(entry->key);
    free(entry);
}

void amap_init(amap_t *map) {
    arraylist_init(&map->entries);
}

void amap_deinit(amap_t *map) {
    arraylist_deinit(&map->entries);
}

void amap_add(amap_t *map, char *key, void *value, free_cb cb) {
    amap_entry_t *entry = 0;
    for (int i = 0; i < map->entries.size; i++) {
        entry = map->entries.nodes[i].data;
        if (strcmp(key, entry->key) == 0) {
            break;
        }
        entry = 0;
    }

    if (!entry) {
        entry = malloc(sizeof(amap_entry_t));
        arraylist_add(&map->entries, entry, amap_free_entry);
    }

    entry->key = strdup(key);
    entry->cb = cb;
    entry->value = value;
}

void amap_rem(amap_t *map, char *key) {
    for (int i = 0; i < map->entries.size; i++) {
        amap_entry_t *entry = map->entries.nodes[i].data;
        if (strcmp(key, entry->key) == 0) {
            arraylist_rem(&map->entries, entry);
            break;
        }
    }
}

void *amap_get(amap_t *map, char *key) {
    for (int i = 0; i < map->entries.size; i++) {
        amap_entry_t *entry = map->entries.nodes[i].data;
        if (strcmp(key, entry->key) == 0) {
            return entry->value;
        }
    }
    return 0;
}