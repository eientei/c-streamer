//
// Created by user on 2/17/17.
//

#ifndef VIDEO_GENERIC_H
#define VIDEO_GENERIC_H

#include <stddef.h>
#include <stdint.h>

typedef void (*free_cb)(void *);

void video_byte_swap_generic(void *var, size_t len);

#define video_byte_swap_int16(value) (uint16_t) (\
                                     ((uint16_t)(value) & 0xFF00) >> 8 \
                                   | ((uint16_t)(value) & 0x00FF) << 8 )

#define video_byte_swap_int32(value) (uint32_t) (\
                                     ((uint32_t)(value) & 0xFF000000) >> 24 \
                                   | ((uint32_t)(value) & 0x000000FF) << 24 \
                                   | ((uint32_t)(value) & 0x00FF0000) >> 8 \
                                   | ((uint32_t)(value) & 0x0000FF00) << 8 )

/*



#define video_byte_swap_int64(value) (((uint64_t)value) & 0xFF00000000000000) >> 56 \
                                   | (((uint64_t)value) & 0x00000000000000FF) << 56 \
                                   | (((uint64_t)value) & 0x00FF000000000000) >> 40 \
                                   | (((uint64_t)value) & 0x000000000000FF00) << 40 \
                                   | (((uint64_t)value) & 0x0000FF0000000000) >> 24 \
                                   | (((uint64_t)value) & 0x0000000000FF0000) << 24 \
                                   | (((uint64_t)value) & 0x000000FF00000000) >> 8 \
                                   | (((uint64_t)value) & 0x00000000FF000000) << 8
*/



#endif //VIDEO_GENERIC_H
