//
// Created by user on 2/16/17.
//

#ifndef VIDEO_MAP_H
#define VIDEO_MAP_H

#include "util/list.h"

typedef void (*free_cb)(void *data);

typedef struct amap_entry_s amap_entry_t;
struct amap_entry_s {
    char *key; ///< map key
    void *value; ///< map value
    free_cb cb; ///< value deallocator
};

typedef struct amap_s amap_t;
struct amap_s {
    arraylist_t entries; ///< map entries
};

/**
 * Initializes a map
 *
 * @param map map to initialize
 */
void amap_init(amap_t *map);

/**
 * Deinitializes a map
 *
 * @param map map to deinitialize
 */
void amap_deinit(amap_t *map);

/**
 * Adds entry to the map
 *
 * @param map map to operate on
 * @param key key to use for lookups
 * @param value value to retrieve by key
 * @param cb value deallocator
 */
void amap_add(amap_t *map, char *key, void *value, free_cb cb);

/**
 * Removes entry from map
 *
 * @param map map to operate on
 * @param key key to remove
 */
void amap_rem(amap_t *map, char *key);

/**
 * Retrieves value from map by key
 *
 * @param map map to operate on
 * @param key key to lookup
 * @return value
 */
void *amap_get(amap_t *map, char *key);

#endif //VIDEO_MAP_H
