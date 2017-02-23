//
// Created by user on 2/17/17.
//

#include <stdlib.h>
#include <stdio.h>
#include "buffer.h"

static void video_pool_free(void *ptr) {
    video_pool_t *pool = ptr;
    video_list_deinit(&pool->buffers);
    uv_mutex_destroy(&pool->mutex);
    free(pool);
}

static void video_buffer_free(void *ptr) {
    video_buffer_t *buffer = ptr;
    buffer->pool = 0;
    free(buffer);
}

void video_slab_init(video_slab_t *slab, size_t min, size_t max, size_t presize) {
    slab->min = min;
    slab->max = max;
    video_list_init(&slab->pools);
    for (size_t i = min; i <= max; i <<= 1) {
        video_pool_t *pool = calloc(1, sizeof(video_pool_t));
        pool->bufsiz = i;
        video_list_init(&pool->buffers);
        video_list_add(&slab->pools, pool, video_pool_free);
        uv_mutex_init(&pool->mutex);

        for (size_t n = 0; n < presize; n++) {
            video_buffer_t *buffer = calloc(1, sizeof(video_buffer_t) + i);
            buffer->pool = pool;
            buffer->refcount = 0;
            video_list_add(&pool->buffers, buffer, video_buffer_free);
        }
    }
}

void video_slab_deinit(video_slab_t *slab) {
    for (size_t i = 0; i < slab->pools.size; i++) {
        video_pool_t *pool = slab->pools.nodes[i].data;
        for (size_t n = 0; n < pool->buffers.size; n++) {
            video_buffer_t *buffer = pool->buffers.nodes[n].data;
            if (buffer->refcount > 0) {
                printf("Unreleased buffer %ld in pool (%ld) %p of slab %p\n", i, pool->bufsiz, pool, slab);
            }
        }
    }
    video_list_deinit(&slab->pools);
}

void *video_slab_alloc(video_slab_t *slab, size_t size) {
    size_t cap = slab->min;
    size_t idx = 0;
    while (cap < size) {
        cap <<= 1;
        idx++;
    }

    if (cap > slab->max) {
        printf("Requested size %ld exceeds limit of %ld\n", size, slab->max);
        return 0;
    }

    video_pool_t *pool = slab->pools.nodes[idx].data;
    uv_mutex_lock(&pool->mutex);
    for (size_t i = 0; i < pool->buffers.size; i++) {
        video_buffer_t *buffer = pool->buffers.nodes[i].data;
        if (buffer->refcount == 0) {
            buffer->refcount = 1;
            uv_mutex_unlock(&pool->mutex);
            return (char*)buffer + sizeof(video_buffer_t);
        }
    }

    video_buffer_t *buffer = calloc(1, sizeof(video_buffer_t) + cap);
    buffer->pool = pool;
    buffer->refcount = 1;
    video_list_add(&pool->buffers, buffer, video_buffer_free);

    uv_mutex_unlock(&pool->mutex);
    return (char*)buffer + sizeof(video_buffer_t);
}

void video_slab_ref(void *ptr) {
    video_buffer_t *buffer = (video_buffer_t *) ((char *) ptr - sizeof(video_buffer_t));
    uv_mutex_lock(&buffer->pool->mutex);
    buffer->refcount++;
    uv_mutex_unlock(&buffer->pool->mutex);
}

void video_slab_unref(void *ptr) {
    if (ptr == 0) {
        return;
    }

    video_buffer_t *buffer = (video_buffer_t *) ((char *) ptr - sizeof(video_buffer_t));
    uv_mutex_lock(&buffer->pool->mutex);
    if (buffer->refcount == 0) {
        printf("Requested freeing already unreferenced buffer %p (%ld)\n", ptr, buffer->pool->bufsiz);
        uv_mutex_unlock(&buffer->pool->mutex);
        return;
    }

    buffer->refcount--;
    uv_mutex_unlock(&buffer->pool->mutex);
}

void video_slab_stats(video_slab_t *slab, char *header) {
    printf("%s slab stats\n", header);
    for (size_t i = 0; i < slab->pools.size; i++) {
        size_t alloc = 0;
        size_t free = 0;
        video_pool_t *pool = slab->pools.nodes[i].data;
        for (size_t n = 0; n < pool->buffers.size; n++) {
            video_buffer_t *buffer = pool->buffers.nodes[n].data;
            if (buffer->refcount) {
                alloc++;
            } else {
                free++;
            }
        }
        printf("buffsiz %10ld, size = %ld, free = %ld, acquire = %ld\n", pool->bufsiz, pool->buffers.size, free, alloc);
    }
};