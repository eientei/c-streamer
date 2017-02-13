//
// Created by user on 2/12/17.
//

#ifndef VIDEO_BUFFER_H
#define VIDEO_BUFFER_H

typedef struct bufslab_s bufslab_t;

struct bufslab_s {
    int min; ///< first power of two
    int max; ///< last power of two
    struct bufpool_s *pools; ///< pools for each power of two inbetween min..max
};

typedef struct bufpool_s bufpool_t;

struct bufpool_s {
    bufslab_t *slab; ///< parental slab
    int capacity; ///< capacity of pooled buffers
    int size; ///< total pool size, both for #released and #acquired
    void **released; ///< allocated, but released buffers
    void **acquired; ///< allocated buffers in use
};

#define buf2base(ptr) ((bufbase_t*)(((char*)(ptr)) - sizeof(bufbase_t)))
#define base2buf(ptr) ((void*)(((char*)(ptr)) + sizeof(bufbase_t)))

typedef struct bufbase_s bufbase_t;

struct bufbase_s {
    bufpool_t *pool; ///< parent pool
    int capacity; ///< capacity of the #data
    int refcount; ///< reference count
};

/**
 * Initalizes a slab
 *
 * @param slab slab to operate on
 * @param min first power of two
 * @param max last power of two
 * @param size pre-allocation factor
 */
void bufslab_init(bufslab_t *slab, int min, int max, int size);

/**
 * Deinitializes the slab
 *
 * @param slab slab to deinitialize
 */
void bufslab_deinit(bufslab_t *slab);

/**
 * Acquire a buffer of at least specified size
 *
 * @param slab slab to allocate on
 * @param size required buffer size
 * @return pointer to a bufer data
 */
void *bufslab_acquire(bufslab_t *slab, int size);

/**
 * Release a buffer acquired earlier
 *
 * @param ptr pointer to buffer returned by #bufslab_acquire
 */
void bufslab_release(void *ptr);

/**
 * Increment a reference-counter of a buffer
 *
 * @param ptr pointer to a buffer returned by #bufslab_acquire
 */
void bufslab_ref(void *ptr);

/**
 * Decrement a reference-counter of a buffer
 *
 * @param ptr pointer to a buffer returned by #bufslab_acquire
 */
void bufslab_unref(void *ptr);

/**
 * Prints a stats for specified slab
 *
 * @param slab slab to print stats of
 */
void bufslab_print(bufslab_t *slab);



/*
typedef struct bufslab_s bufslab_t;

struct bufslab_s {
    int min; // shortest buffer size
    int max; // longest buffer size
    int limit; // maximum size of pools buffers combined
    struct bufpool_s* pools; // pools for each buffer size with step of next power of two (128, 256, 512, etc.)
};

typedef struct bufpool_s bufpool_t;

struct bufpool_s {
    int capacity; // pool capacity
    int power; // pool buffers' capacity
    int cursor; // pool last element cursor
    bufslab_t *slab; // parent slab
    void **buffs; // allocated buffers
};

#define bufbase(ptr) ((bufbase_t*)((char*)(ptr)) - sizeof(bufbase_t))
#define buflen(ptr) (bufbase(ptr)->len)

typedef struct bufbase_s bufbase_t;

struct bufbase_s {
    bufpool_t *pool; // buffer parent pool
    int len; // buffer length
};

// interface
void  bufslab_init(bufslab_t *slab, int min, int max, int limit);
void  bufslab_deinit(bufslab_t *slab);
void *bufslab_acquire(bufslab_t *slab, int len);
void  bufslab_release(void *ptr);
void  bufslab_print_stats(bufslab_t *slab);


// internal implementation-specific
void  bufpool_init(bufpool_t *pool, bufslab_t *slab, int power, int capacity);
void  bufpool_deinit(bufpool_t *pool);
void *bufpool_alloc(bufpool_t *pool);
void  bufpool_free(void *ptr);
void *bufpool_dequeue(bufpool_t *pool);
void  bufpool_enqueue(bufpool_t *pool, void *ptr);
void  bufpool_grow(bufpool_t *pool);
*/

#endif //VIDEO_BUFFER_H
