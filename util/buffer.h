//
// Created by user on 2/17/17.
//

#ifndef VIDEO_BUFFER_H
#define VIDEO_BUFFER_H

#include "util/generic.h"
#include "util/list.h"

typedef struct video_slab_s video_slab_t;
struct video_slab_s {
    size_t min;
    size_t max;
    video_list_t pools;
};

typedef struct video_pool_s video_pool_t;
struct video_pool_s {
    size_t bufsiz;
    video_list_t buffers;
};

typedef struct video_buffer_s video_buffer_t;
struct video_buffer_s {
    video_pool_t *pool;
    size_t refcount;
};

void video_slab_init(video_slab_t *slab, size_t min, size_t max, size_t presize);
void video_slab_deinit(video_slab_t *slab);
void *video_slab_alloc(video_slab_t *slab, size_t size);
void video_slab_ref(void *ptr);
void video_slab_unref(void *ptr);
void video_slab_stats(video_slab_t *slab, char *header);

#endif //VIDEO_BUFFER_H
