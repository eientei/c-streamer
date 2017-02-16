//
// Created by user on 2/12/17.
//

#ifndef VIDEO_RTMP_H
#define VIDEO_RTMP_H

#include <uv.h>
#include "util/list.h"

#define RTMP_MAX_HEADER_SIZE 128
#define RTMP_MAX_MESSAGE_SIZE 65536
#define RTMP_BUFFER_SIZE 2048

typedef enum rtmp_message_type_e rtmp_message_type;
enum rtmp_message_type_e {
    RTMP_MESSAGE_TYPE_0,
    RTMP_MESSAGE_TYPE_SET_CHUNK_SIZE,
    RTMP_MESSAGE_TYPE_ABORT,
    RTMP_MESSAGE_TYPE_ACKNOWLEDGEMENT,
    RTMP_MESSAGE_TYPE_USER_CONTROL,
    RTMP_MESSAGE_TYPE_WINDOW_ACKNOWLEDGEMENT_SIZE,
    RTMP_MESSAGE_TYPE_SET_PEER_BANDWIDTH,
    RTMP_MESSAGE_TYPE_7,
    RTMP_MESSAGE_TYPE_AUDIO,
    RTMP_MESSAGE_TYPE_VIDEO,
    RTMP_MESSAGE_TYPE_10,
    RTMP_MESSAGE_TYPE_AMF3_CMD_ALT,
    RTMP_MESSAGE_TYPE_12,
    RTMP_MESSAGE_TYPE_13,
    RTMP_MESSAGE_TYPE_14,
    RTMP_MESSAGE_TYPE_AMF3_META,
    RTMP_MESSAGE_TYPE_16,
    RTMP_MESSAGE_TYPE_AMF3_CMD,
    RTMP_MESSAGE_TYPE_AMF0_META,
    RTMP_MESSAGE_TYPE_19,
    RTMP_MESSAGE_TYPE_AMF0_CMD
};
static char *rtmp_message_type_names[] = {
        "UNKNOWN_MESSAGE_TYPE_0",
        "SET_CHUNK_SIZE",
        "ABORT",
        "ACKNOWLEDGEMENT",
        "USER_CONTROL",
        "WINDOW_ACKNOWLEDGEMENT_SIZE",
        "SET_PEER_BANDWIDTH",
        "UNKNOWN_MESSAGE_TYPE_7",
        "AUDIO",
        "VIDEO",
        "UNKNOWN_MESSAGE_TYPE_10",
        "AMF3_CMD_ALT",
        "UNKNOWN_MESSAGE_TYPE_12",
        "UNKNOWN_MESSAGE_TYPE_13",
        "UNKNOWN_MESSAGE_TYPE_14",
        "AMF3_META",
        "UNKNOWN_MESSAGE_TYPE_16",
        "AMF3_CMD",
        "AMF0_META",
        "UNKNOWN_MESSAGE_TYPE_19",
        "AMF0_CMD"
};

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

typedef struct rtmp_chunk_s rtmp_chunk_t;
struct rtmp_chunk_s {
    int chunkid;

    int timestamp;
    int length;
    rtmp_message_type type;
    int stream;

    uv_buf_t buffer;
};

typedef struct rtmp_context_s rtmp_context_t;
struct rtmp_context_s {
    uv_handle_t *stream;
    rtmp_state state;
    rtmp_kind kind;

    arraylist_t chunks;
    rtmp_chunk_t *header;

    size_t inchunk;
    size_t outchunk;

    uv_buf_t handshakebuf;
    uv_buf_t headerbuf;
    uv_buf_t messagebuf;
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
