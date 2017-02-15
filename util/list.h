//
// Created by user on 2/14/17.
//

#ifndef VIDEO_LIST_H
#define VIDEO_LIST_H

typedef enum arrayaction_e arrayaction;
enum arrayaction_e {
    ARRAY_ACTION_CONTINUE,
    ARRAY_ACTION_DELETE,
    ARRAY_ACTION_STOP
};

typedef void (*free_cb)(void *data);
typedef arrayaction (*walk_cb)(void *data, void *arg);

typedef struct arraynode_s arraynode_t;
struct arraynode_s {
    void *data; ///< payload data
    free_cb cb; ///< free callback
};

typedef struct arraylist_s arraylist_t;
struct arraylist_s {
    int capacity; ///< capcity of array
    int size; ///< current size of array
    arraynode_t *nodes; ///< nodes list
};

/**
 * Initializes an array
 *
 * @param list list to initialize
 */
void arraylist_init(arraylist_t *list);

/**
 * Deinitializes an array
 *
 * @param list list to deinitialize
 */
void arraylist_deinit(arraylist_t *list);

/**
 * Adds and element with optional destructor
 *
 * @param list list to add to
 * @param data data to add
 * @param cb data destructor
 */
void arraylist_add(arraylist_t *list, void *data, free_cb cb);

/**
 * Removes an element from the list
 *
 * @param list list to remove from
 * @param data data to remove
 */
void arraylist_rem(arraylist_t *list, void *data);

/**
 * Retrieves an element from list
 *
 * @param list list to get from
 * @param idx index to get from
 * @return stored data
 */
void *arraylist_get(arraylist_t *list, int idx);

/**
 * Finds index of specified element
 *
 * @param list to search on
 * @param data to searrch for
 * @return index
 */
int arraylist_indexof(arraylist_t *list, void *data);

/**
 * Walks a list with spcified callback
 *
 * @param list to walk on
 * @param walker to walk with
 * @param data context for the walker
 */
void arraylist_walk(arraylist_t *list, walk_cb walker, void *data);

#endif //VIDEO_LIST_H
