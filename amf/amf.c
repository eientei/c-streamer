//
// Created by user on 2/20/17.
//

#include <string.h>
#include <stdlib.h>
#include "amf.h"

static size_t video_amf_decode_entry(video_amf_value_t *value, void *data, size_t len) {
    char *ptr = data;
    size_t slen = 0;
    slen |= *ptr++ << 8;
    slen |= *ptr++;
    char *key = calloc(1, slen+1);
    memmove(key, ptr, slen);
    ptr += slen;
    video_amf_value_t *child = calloc(1, sizeof(video_amf_value_t));
    ptr += video_amf_decode(child, ptr, len - (ptr - (char*) data));
    video_map_put(&value->as.map, key, child, video_amf_value_free);
    free(key);

    return ptr - (char*) data;
}

static size_t video_amf_estimate_map(video_amf_value_t *value) {
    size_t total = 0;
    for (size_t i = 0; i < value->as.map.size; i++) {
        video_map_entry_t *entry = value->as.map.nodes[i].data;
        total += 2 + strlen(entry->key);
        total += video_amf_estimate(entry->value);
    }
    return total;
}

static size_t video_amf_encode_map(video_amf_value_t *value, void *data) {
    char *tgt = data;
    uint16_t size16;
    for (size_t i = 0; i < value->as.map.size; i++) {
        video_map_entry_t *entry = value->as.map.nodes[i].data;
        size16 = video_byte_swap_int16((uint16_t) strlen(entry->key));
        memmove(tgt, &size16, 2);
        tgt += 2;
        memmove(tgt, entry->key, video_byte_swap_int16(size16));
        tgt += video_byte_swap_int16(size16);
        tgt += video_amf_encode(entry->value, tgt);
    }
    return tgt - (char*) data;
}

size_t video_amf_decode(video_amf_value_t *value, void *data, size_t len) {
    char *ptr = data;
    uint32_t size32 = 0;
    uint32_t size16 = 0;
    value->type = (video_amf_type) *ptr++;
    switch (value->type) {
        case VIDEO_AMF_TYPE_NUMBER:
            memmove(&value->as.dbl, ptr, 8);
            ptr += 8;
            video_byte_swap_generic(&value->as.dbl, 8);
            break;
        case VIDEO_AMF_TYPE_BOOLEAN:
            value->as.bool = *ptr++;
            break;
        case VIDEO_AMF_TYPE_LONGSTRING:
            size32 |= *ptr++ << 24;
            size32 |= *ptr++ << 16;
            size32 |= *ptr++ << 8;
            size32 |= *ptr++;
            value->as.str = calloc(1, size32+1);
            memmove(value->as.str, ptr, size32);
            ptr += size32;
            break;
        case VIDEO_AMF_TYPE_STRING:
            size16 |= *ptr++ << 8;
            size16 |= *ptr++;
            value->as.str = calloc(1, size16+1);
            memmove(value->as.str, ptr, size16);
            ptr += size16;
            break;
        case VIDEO_AMF_TYPE_OBJECT:
            video_map_init(&value->as.map);
            while (ptr - (char*) data < len) {
                if (*(ptr) == 0 && *(ptr+1) == 0 && *(ptr+2) == VIDEO_AMF_TYPE_OBJECTEND) {
                    ptr += 3;
                    break;
                }
                ptr += video_amf_decode_entry(value, ptr, len - (ptr - (char*) data));
            }
            break;
        case VIDEO_AMF_TYPE_ECMAARRAY:
            video_map_init(&value->as.map);
            memmove(&size32, ptr, 4);
            ptr += 4;
            for (size_t i = 0; i < size32; i++) {
                ptr += video_amf_decode_entry(value, ptr, len - (ptr - (char*) data));
            }

            break;
        case VIDEO_AMF_TYPE_STRICTARRAY:
            video_list_init(&value->as.list);
            memmove(&size32, ptr, 4);
            ptr += 4;
            for (size_t i = 0; i < size32; i++) {
                video_amf_value_t *child = calloc(1, sizeof(video_amf_value_t));
                ptr += video_amf_decode(child, ptr, len - (ptr - (char*) data));
                video_list_add(&value->as.list, child, video_amf_value_free);
            }
            break;
        default:
            break;
    }


    return ptr - (char*) data;
}

size_t video_amf_decodeall(video_list_t *list, void *data, size_t len) {
    char *ptr = data;
    while (ptr - (char*)data < len) {
        video_amf_value_t *value = calloc(1, sizeof(video_amf_value_t));
        ptr += video_amf_decode(value, ptr, len - (ptr - (char*) data));
        video_list_add(list, value, video_amf_value_free);
    }
    return ptr - (char*) data;
}

void video_amf_value_free(void *ptr) {
    video_amf_value_t *value = ptr;
    switch (value->type) {
        case VIDEO_AMF_TYPE_LONGSTRING:
        case VIDEO_AMF_TYPE_STRING:
            free(value->as.str);
            break;
        case VIDEO_AMF_TYPE_OBJECT:
        case VIDEO_AMF_TYPE_ECMAARRAY:
            video_map_deinit(&value->as.map);
            break;
        case VIDEO_AMF_TYPE_STRICTARRAY:
            video_list_deinit(&value->as.list);
            break;
        default:
            break;
    }
    free(value);
}

size_t video_amf_estimate(video_amf_value_t *value) {
    size_t total = 1;
    switch (value->type) {
        case VIDEO_AMF_TYPE_NUMBER:
            total += 8;
            break;
        case VIDEO_AMF_TYPE_BOOLEAN:
            total += 1;
            break;
        case VIDEO_AMF_TYPE_LONGSTRING:
            total += 4 + strlen(value->as.str);
            break;
        case VIDEO_AMF_TYPE_STRING:
            total += 2 + strlen(value->as.str);
            break;
        case VIDEO_AMF_TYPE_OBJECT:
            total += video_amf_estimate_map(value);
            total += 3;
            break;
        case VIDEO_AMF_TYPE_ECMAARRAY:
            total += 4;
            total += video_amf_estimate_map(value);
            break;
        case VIDEO_AMF_TYPE_STRICTARRAY:
            total += 4;
            for (size_t i = 0; i < value->as.list.size; i++) {
                total += video_amf_estimate(value->as.list.nodes[i].data);
            }
            break;
        default:
            break;
    }

    return total;
}

size_t video_amf_encode(video_amf_value_t *value, void *data) {
    char *tgt = data;
    uint16_t size16;
    uint32_t size32;
    *tgt++ = value->type;
    double dbl;
    switch (value->type) {
        case VIDEO_AMF_TYPE_NUMBER:
            dbl = value->as.dbl;
            video_byte_swap_generic(&dbl, 8);
            memmove(tgt, &dbl, 8);
            tgt += 8;
            break;
        case VIDEO_AMF_TYPE_BOOLEAN:
            *tgt++ = value->as.bool;
            break;
        case VIDEO_AMF_TYPE_LONGSTRING:
            size32 = video_byte_swap_int32((uint32_t) strlen(value->as.str));
            memmove(tgt, &size32, 4);
            tgt += 4;
            memmove(tgt, value->as.str, video_byte_swap_int32(size32));
            tgt += video_byte_swap_int32(size32);
            break;
        case VIDEO_AMF_TYPE_STRING:
            size16 = video_byte_swap_int16((uint16_t) strlen(value->as.str));
            memmove(tgt, &size16, 2);
            tgt += 2;
            memmove(tgt, value->as.str, video_byte_swap_int16(size16));
            tgt += video_byte_swap_int16(size16);
            break;
        case VIDEO_AMF_TYPE_OBJECT:
            tgt += video_amf_encode_map(value, tgt);
            *tgt++ = 0;
            *tgt++ = 0;
            *tgt++ = VIDEO_AMF_TYPE_OBJECTEND;
            break;
        case VIDEO_AMF_TYPE_ECMAARRAY:
            size32 = video_byte_swap_int32((uint32_t) value->as.map.size);
            memmove(tgt, &size32, 4);
            tgt += 4;
            tgt += video_amf_encode_map(value, tgt);
            break;
        case VIDEO_AMF_TYPE_STRICTARRAY:
            size32 = video_byte_swap_int32((uint32_t) value->as.list.size);
            memmove(tgt, &size32, 4);
            for (size_t i = 0; i < value->as.list.size; i++) {
                tgt += video_amf_encode(value->as.list.nodes[i].data, tgt);
            }
            break;
        default:
            break;
    }
    return tgt - (char*) data;
}

video_amf_value_t *video_amf_make_number(double value) {
    video_amf_value_t *amfvalue = calloc(1, sizeof(video_amf_value_t));
    amfvalue->type = VIDEO_AMF_TYPE_NUMBER;
    amfvalue->as.dbl = value;
    return amfvalue;
}

video_amf_value_t *video_amf_make_bool(char value) {
    video_amf_value_t *amfvalue = calloc(1, sizeof(video_amf_value_t));
    amfvalue->type = VIDEO_AMF_TYPE_BOOLEAN;
    amfvalue->as.bool = value;
    return amfvalue;
}
video_amf_value_t *video_amf_make_string(char *value) {
    video_amf_value_t *amfvalue = calloc(1, sizeof(video_amf_value_t));
    amfvalue->type = VIDEO_AMF_TYPE_STRING;
    amfvalue->as.str = strdup(value);
    return amfvalue;
}
video_amf_value_t *video_amf_make_object() {
    video_amf_value_t *amfvalue = calloc(1, sizeof(video_amf_value_t));
    amfvalue->type = VIDEO_AMF_TYPE_OBJECT;
    video_map_init(&amfvalue->as.map);
    return amfvalue;
}

video_amf_value_t *video_amf_make_ecmaarray() {
    video_amf_value_t *amfvalue = calloc(1, sizeof(video_amf_value_t));
    amfvalue->type = VIDEO_AMF_TYPE_ECMAARRAY;
    video_map_init(&amfvalue->as.map);
    return amfvalue;
}
video_amf_value_t *video_amf_make_strictarray() {
    video_amf_value_t *amfvalue = calloc(1, sizeof(video_amf_value_t));
    amfvalue->type = VIDEO_AMF_TYPE_ECMAARRAY;
    video_map_init(&amfvalue->as.map);
    return amfvalue;
}
video_amf_value_t *video_amf_make_null() {
    video_amf_value_t *amfvalue = calloc(1, sizeof(video_amf_value_t));
    amfvalue->type = VIDEO_AMF_TYPE_NULL;
    return amfvalue;
}