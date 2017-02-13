//
// Created by user on 2/12/17.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "buffer.h"

static void bufslab_buf_init(bufpool_t *pool, bufbase_t *buf) {
    buf->capacity = pool->capacity;
    buf->pool = pool;
}

static void bufslab_buf_deinit(void *ptr) {
    free(buf2base(ptr));
}

static void bufslab_pool_init(bufpool_t *pool, int size, int capacity, bufslab_t *slab) {
    pool->size = size;
    pool->slab = slab;
    pool->capacity = capacity;
    pool->acquired = calloc((size_t) size, sizeof(void*));
    pool->released = calloc((size_t) size, sizeof(void*));
}

static void bufslab_pool_deinit(bufpool_t *pool) {
    for (int i = 0; i < pool->size; i++) {
        if (pool->released[i]) {
            free(buf2base(pool->released[i]));
        }
        if (pool->acquired[i]) {
            fprintf(stderr, "Unreleased buffer %d in pool (%d) %p of slab %p\n", i, pool->capacity, pool, pool->slab);
        }
    }
    free(pool->acquired);
    free(pool->released);
}

static void bufslab_pool_grow(bufpool_t *pool) {
    pool->size <<= 1;
    pool->released = realloc(pool->released, sizeof(void*) * pool->size);
    pool->acquired = realloc(pool->acquired, sizeof(void*) * pool->size);
    for (int i = pool->size >> 1; i < pool->size; i++) {
        pool->released[i] = 0;
        pool->acquired[i] = 0;
    }
}

void bufslab_init(bufslab_t *slab, int min, int max, int size) {
    slab->min = min;
    slab->max = max;

    int count = 1;
    for (int p = min; p < max; p <<= 1) {
        count++;
    }
    slab->pools = malloc(sizeof(bufpool_t) * count);

    count = 0;
    for (int p = min; p <= max; p <<= 1) {
        bufslab_pool_init(&slab->pools[count], size, p, slab);
        count++;
    }
}

void bufslab_deinit(bufslab_t *slab) {
    int count = 0;
    for (int i = slab->min; i <= slab->max; i <<= 1) {
        bufslab_pool_deinit(&slab->pools[count]);
        count++;
    }
    free(slab->pools);
}

void *bufslab_acquire(bufslab_t *slab, int size) {
    if (size > slab->max) {
        fprintf(stderr, "Requested too big buffer, maximum capacity: %d, requested: %d\n", slab->max, size);
        return 0;
    }

    int count = 0;
    for (int i = slab->min; i < size; i <<= 1) {
        count++;
    }

    bufpool_t *pool = &slab->pools[count];
    for (int i = 0; i < pool->size; i++) {
        if (pool->released[i]) {
            for (int n = 0; n < pool->size; n++) {
                if (!pool->acquired[n]) {
                    pool->acquired[n] = pool->released[i];;
                    pool->released[i] = 0;
                    buf2base(pool->acquired[i])->refcount = 1;
                    return pool->acquired[i];
                }
            }
        }
    }
    for (int i = 0; i < pool->size; i++) {
        if (!pool->acquired[i]) {
            pool->acquired[i] = base2buf(malloc(sizeof(bufbase_t) + pool->capacity));
            bufslab_buf_init(pool, buf2base(pool->acquired[i]));
            buf2base(pool->acquired[i])->refcount = 1;
            return pool->acquired[i];
        }
    }
    bufslab_pool_grow(pool);
    return bufslab_acquire(slab, size);
}

void bufslab_ref(void *ptr) {
    buf2base(ptr)->refcount += 1;
}

void bufslab_unref(void *ptr) {
    bufbase_t *base = buf2base(ptr);
    base->refcount -= 1;
    if (base->refcount == 0) {
        bufslab_release(ptr);
    }
}

void bufslab_release(void *ptr) {
    bufbase_t *base = buf2base(ptr);
    bufpool_t *pool = base->pool;
    for (int i = 0; i < pool->size; i++) {
        if (pool->acquired[i] == ptr) {
            for (int n = 0; n < pool->size; n++) {
                if (!pool->released[n]) {
                    pool->released[n] = pool->acquired[i];
                    pool->acquired[i] = 0;
                    return;
                }
            }
        }
    }
}

void bufslab_print(bufslab_t *slab) {

}