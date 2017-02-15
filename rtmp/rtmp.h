//
// Created by user on 2/12/17.
//

#ifndef VIDEO_RTMP_H
#define VIDEO_RTMP_H

#include <uv.h>
#include "util/list.h"

typedef enum rtmp_state_e rtmp_state;
enum rtmp_state_e {
    RTMP_STATE_S0,
    RTMP_STATE_S1,
    RTMP_STATE_S2,
    RTMP_STATE_HEADER,
    RTMP_STATE_BODY
};

typedef enum rtmp_kind_e rtmp_kind;
enum rtmp_kind_e {
    RTMP_KIND_UNKNOWN,
    RTMP_KIND_PUB,
    RTMP_KIND_SUB
};

typedef struct rtmp_chunk_s rtmp_chunk;
struct rtmp_chunk_s {
    int chunkid;

    int timestamp;
    int length;
    char type;
    int stream;

    void *buffer;
};

typedef struct rtmp_context_s rtmp_context_t;
struct rtmp_context_s {
    uv_handle_t *stream;
    rtmp_state state;
    rtmp_kind kind;

    arraylist_t chunks;

    int inchunk;
    int outchunk;

    uv_buf_t handshakebuf;
};

typedef struct rtmp_channel_s rtmp_channel_t;
struct rtmp_channel_s {
    char *name;

    rtmp_context_t *publisher;

    arraylist_t subscribers;
};

typedef struct rtmp_global_s rtmp_global_t;
struct rtmp_global_s {
    arraylist_t channels;
    arraylist_t contexts;
};

void rtmp_init();
void rtmp_connected(uv_stream_t* stream);
void rtmp_disconnect(uv_handle_t *stream);
void rtmp_disconnected(uv_handle_t* stream);
void rtmp_deinit();

#endif //VIDEO_RTMP_H
