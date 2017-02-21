//
// Created by user on 2/17/17.
//

#ifndef VIDEO_MAP_H
#define VIDEO_MAP_H

#include "util/generic.h"
#include "util/list.h"

typedef struct video_map_entry_s video_map_entry_t;
struct video_map_entry_s {
    char *key;
    void *value;
    free_cb cb;
};

typedef video_list_t video_map_t;

void video_map_init(video_map_t *map);
void video_map_deinit(video_map_t *map);
void video_map_put(video_map_t *map, char *key, void *value, free_cb cb);
void video_map_remove(video_map_t *map, char *key);
void *video_map_get(video_map_t *map, char *key);

#endif //VIDEO_MAP_H
