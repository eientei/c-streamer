//
// Created by user on 2/15/17.
//

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "amf.h"

static void amf_freevalue(void *ptr) {
    amf_value_t *value = ptr;
    switch (value->type) {
        case AMF_TYPE_LONGSTRING:
        case AMF_TYPE_STRING:
            free(value->as.str);
            break;
        case AMF_TYPE_OBJECT:
        case AMF_TYPE_ECMAARRAY:
            amap_deinit(&value->as.map);
            break;
        case AMF_TYPE_STRICTARRAY:
            arraylist_deinit(&value->as.list);
            break;
        default:
            break;
    }
    free(value);
}

static void amf_byteswapnum(void *data) {
    uint64_t num = *(uint64_t*)data;
    num = (num & 0x00000000FFFFFFFF) << 32 | (num & 0xFFFFFFFF00000000) >> 32;
    num = (num & 0x0000FFFF0000FFFF) << 16 | (num & 0xFFFF0000FFFF0000) >> 16;
    num = (num & 0x00FF00FF00FF00FF) << 8  | (num & 0xFF00FF00FF00FF00) >> 8;
    *(uint64_t*)data = num;
}

static size_t amf_readvalue(amf_value_t *value, char *data, size_t remain) {
    char *ptr = data;
    value->type = (amf_type) *ptr++;
    size_t slen = 0;
    switch (value->type) {
        case AMF_TYPE_NUMBER:
            memmove(&value->as.dbl, ptr, 8);
            ptr += 8;
            amf_byteswapnum(&value->as.dbl);
            break;
        case AMF_TYPE_BOOLEAN:
            value->as.bool = *ptr++;
            break;
        case AMF_TYPE_LONGSTRING:
            slen += *ptr++ << 24;
            slen += *ptr++ << 16;
        case AMF_TYPE_STRING:
            slen += *ptr++ << 8;
            slen += *ptr++;
            value->as.str = calloc(1, slen + 1);
            memmove(value->as.str, ptr, slen);
            ptr += slen;
            break;
        case AMF_TYPE_OBJECT:
            amap_init(&value->as.map);
            while (ptr - data < remain) {
                slen = 0;
                slen += *ptr++ << 8;
                slen += *ptr++;
                if (slen == 0 && *ptr == AMF_TYPE_OBJECTEND) {
                    ptr++;
                    break;
                }
                char *key = calloc(1, slen + 1);
                memmove(key, ptr, slen);
                ptr += slen;
                amf_value_t *subvalue = malloc(sizeof(amf_value_t));
                ptr += amf_readvalue(subvalue, ptr, remain - (ptr - data));
                amap_add(&value->as.map, key, subvalue, amf_freevalue);
                free(key);
            }
            break;
        case AMF_TYPE_ECMAARRAY:
            amap_init(&value->as.map);
            int elen;
            memmove(&elen, ptr, 4);
            ptr += 4;
            for (int i = 0; i < elen; i++) {
                slen = 0;
                slen += *ptr++ << 8;
                slen += *ptr++;
                char *key = calloc(1, slen + 1);
                memmove(key, ptr, slen);
                ptr += slen;
                amf_value_t *subvalue = malloc(sizeof(amf_value_t));
                ptr += amf_readvalue(subvalue, ptr, remain - (ptr - data));
                amap_add(&value->as.map, key, subvalue, amf_freevalue);
                free(key);
            }
            break;
        case AMF_TYPE_STRICTARRAY:
            arraylist_init(&value->as.list);
            int alen;
            memmove(&alen, ptr, 4);
            ptr += 4;
            for (int i = 0; i < alen; i++) {
                amf_value_t *subvalue = malloc(sizeof(amf_value_t));
                ptr += amf_readvalue(subvalue, ptr, remain - (ptr - data));
                arraylist_add(&value->as.list, subvalue, amf_freevalue);
            }
            break;
        default:
            break;
    }
    return ptr - data;
}

void amf_array_decode(amf_array_t *amf, void *data, size_t len) {
    arraylist_init(amf);
    char *ptr = data;

    while ((ptr - (char*) data) < len) {
        amf_value_t *value = calloc(1, sizeof(amf_value_t));
        ptr += amf_readvalue(value, ptr, len - (ptr - (char *)data));
        arraylist_add(amf, value, amf_freevalue);
    }
}

void amf_array_free(amf_array_t *amf) {
    arraylist_deinit(amf);
}

size_t amf_array_encode(amf_value_t *value, void *data) {
    char *ptr = data;

    *ptr++ = value->type;
    size_t slen = 0;
    int size = 0;
    switch (value->type) {
        case AMF_TYPE_NUMBER:
            amf_byteswapnum(&value->as.dbl);
            memmove(ptr, &value->as.dbl, 8);
            ptr += 8;
            break;
        case AMF_TYPE_BOOLEAN:
            *ptr++ = value->as.bool;
            break;
        case AMF_TYPE_LONGSTRING:
            slen = strlen(value->as.str);
            *ptr++ = (char) ((slen >> 24) & 0xFF);
            *ptr++ = (char) ((slen >> 16) & 0xFF);
            *ptr++ = (char) ((slen >> 8) & 0xFF);
            *ptr++ = (char) (slen & 0xFF);
            memmove(ptr, value->as.str, slen);
            ptr += slen;
            break;
        case AMF_TYPE_STRING:
            slen = strlen(value->as.str);
            *ptr++ = (char) ((slen >> 8) & 0xFF);
            *ptr++ = (char) (slen & 0xFF);
            memmove(ptr, value->as.str, slen);
            ptr += slen;
            break;
        case AMF_TYPE_OBJECT:
            for (int i = 0; i < value->as.map.entries.size; i++) {
                amap_entry_t *entry = value->as.map.entries.nodes[i].data;

                slen = strlen(entry->key);
                *ptr++ = (char) ((slen >> 8) & 0xFF);
                *ptr++ = (char) (slen & 0xFF);
                memmove(ptr, entry->key, slen);
                ptr += slen;
                ptr += amf_array_encode(entry->value, ptr);
            }
            *ptr++ = 0;
            *ptr++ = 0;
            *ptr++ = AMF_TYPE_OBJECTEND;
            break;
        case AMF_TYPE_ECMAARRAY:
            size = value->as.map.entries.size;
            memmove(ptr, &size, 4);
            ptr += 4;
            for (int i = 0; i < size; i++) {
                amap_entry_t *entry = value->as.map.entries.nodes[i].data;
                slen = strlen(entry->key);
                *ptr++ = (char) ((slen >> 8) & 0xFF);
                *ptr++ = (char) (slen & 0xFF);
                memmove(ptr, entry->key, slen);
                ptr += slen;
                ptr += amf_array_encode(entry->value, ptr);
            }
            break;
        case AMF_TYPE_STRICTARRAY:
            memmove(ptr, &value->as.list.size, 4);
            ptr += 4;
            for (int i = 0; i < value->as.list.size; i++) {
                ptr += amf_array_encode(value->as.list.nodes[i].data, ptr);
            }
            break;
        default:
            break;
    }
    return ptr - (char*) data;
}