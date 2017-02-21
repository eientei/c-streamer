//
// Created by user on 2/20/17.
//

#ifndef VIDEO_AMF_H
#define VIDEO_AMF_H

#include "util/list.h"
#include "util/map.h"

typedef enum video_amf_type_e video_amf_type;
enum video_amf_type_e {
    VIDEO_AMF_TYPE_NUMBER,
    VIDEO_AMF_TYPE_BOOLEAN,
    VIDEO_AMF_TYPE_STRING,
    VIDEO_AMF_TYPE_OBJECT,
    VIDEO_AMF_TYPE_MOVIECLIP,
    VIDEO_AMF_TYPE_NULL,
    VIDEO_AMF_TYPE_UNDEFINED,
    VIDEO_AMF_TYPE_REFERENCE,
    VIDEO_AMF_TYPE_ECMAARRAY,
    VIDEO_AMF_TYPE_OBJECTEND,
    VIDEO_AMF_TYPE_STRICTARRAY,
    VIDEO_AMF_TYPE_DATE,
    VIDEO_AMF_TYPE_LONGSTRING,
    VIDEO_AMF_TYPE_UNSUPPORTED,
    VIDEO_AMF_TYPE_RECORDSET,
    VIDEO_AMF_TYPE_XMLDOC,
    VIDEO_AMF_TYPE_TYPEDOBJECT
};

static char *video_amf_type_names[] = {
        "NUMBER",
        "BOOLEAN",
        "STRING",
        "OBJECT",
        "MOVIECLIP",
        "NULL",
        "UNDEFINED",
        "REFERENCE",
        "ECMAARRAY",
        "OBJECTEND",
        "STRICTARRAY",
        "DATE",
        "LONGSTRING",
        "UNSUPPORTED",
        "RECORDSET",
        "XMLDOC",
        "TYPEDOBJECT"
};

union video_amf_value_u {
    double dbl;
    char bool;
    char *str;
    video_list_t list;
    video_map_t map;
};

typedef struct video_amf_value_s video_amf_value_t;
struct video_amf_value_s {
    video_amf_type type;
    union video_amf_value_u as;
};

size_t video_amf_decode(video_amf_value_t *value, void *data, size_t len);
size_t video_amf_decodeall(video_list_t *list, void *data, size_t len);
void video_amf_value_free(void *ptr);
size_t video_amf_estimate(video_amf_value_t *value);
size_t video_amf_encode(video_amf_value_t *value, void *data);

video_amf_value_t *video_amf_make_number(double value);
video_amf_value_t *video_amf_make_bool(char value);
video_amf_value_t *video_amf_make_string(char *value);
video_amf_value_t *video_amf_make_object();
video_amf_value_t *video_amf_make_ecmaarray();
video_amf_value_t *video_amf_make_strictarray();
video_amf_value_t *video_amf_make_null();

#endif //VIDEO_AMF_H
