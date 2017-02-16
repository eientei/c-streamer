//
// Created by user on 2/15/17.
//

#ifndef VIDEO_AMF_H
#define VIDEO_AMF_H

#include "util/list.h"
#include "util/map.h"

typedef enum amf_type_e amf_type;
enum amf_type_e {
    AMF_TYPE_NUMBER,
    AMF_TYPE_BOOLEAN,
    AMF_TYPE_STRING,
    AMF_TYPE_OBJECT,
    AMF_TYPE_MOVIECLIP,
    AMF_TYPE_NULL,
    AMF_TYPE_UNDEFINED,
    AMF_TYPE_REFERENCE,
    AMF_TYPE_ECMAARRAY,
    AMF_TYPE_OBJECTEND,
    AMF_TYPE_STRICTARRAY,
    AMF_TYPE_DATE,
    AMF_TYPE_LONGSTRING,
    AMF_TYPE_UNSUPPORTED,
    AMF_TYPE_RECORDSET,
    AMF_TYPE_XMLDOC,
    AMF_TYPE_TYPEDOBJECT
};

static char *amf_type_names[] = {
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

union as {
    double dbl;
    char bool;
    char *str;
    arraylist_t list;
    amap_t map;
    void *ptr;
};

typedef struct amf_value_s amf_value_t;
struct amf_value_s {
    amf_type type;
    union as as;
};

typedef arraylist_t amf_array_t;

void amf_array_decode(amf_array_t *amf, void *data, size_t len);
void amf_array_free(amf_array_t *amf);
size_t amf_array_encode(amf_value_t *value, void *data);
size_t amf_array_estimate(amf_value_t *value);

#endif //VIDEO_AMF_H
