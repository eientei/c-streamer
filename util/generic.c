//
// Created by user on 2/21/17.
//

#include "util/generic.h"

void video_byte_swap_generic(void *var, size_t len) {
    char *ptr = var;
    char tmp;
    for (size_t i = 0; i < len / 2; i++) {
        tmp = ptr[i];
        ptr[i] = ptr[len-i-1];
        ptr[len-i-1] = tmp;
    }
}