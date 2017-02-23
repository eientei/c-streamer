//
// Created by user on 2/18/17.
//

#ifndef VIDEO_RTMP_H
#define VIDEO_RTMP_H

#include <uv.h>
#include <channel/channel.h>
#include "global/global.h"

    #define VIDEO_RTMP_MESSAGE_MAX 2097152

typedef struct video_rtmp_global_s video_rtmp_global_t;
struct video_rtmp_global_s {
    uv_tcp_t listener;

    video_list_t clients;
};

typedef enum video_rtmp_msgtype_e video_rtmp_msgtype;
enum video_rtmp_msgtype_e {
    VIDEO_RTMP_MSGTYPE_0,
    VIDEO_RTMP_MSGTYPE_SET_CHUNK_SIZE,
    VIDEO_RTMP_MSGTYPE_ABORT,
    VIDEO_RTMP_MSGTYPE_ACKNOWLEDGEMENT,
    VIDEO_RTMP_MSGTYPE_USER_CONTROL,
    VIDEO_RTMP_MSGTYPE_WINDOW_ACKNOWLEDGEMENT_SIZE,
    VIDEO_RTMP_MSGTYPE_SET_PEER_BANDWIDTH,
    VIDEO_RTMP_MSGTYPE_7,
    VIDEO_RTMP_MSGTYPE_AUDIO,
    VIDEO_RTMP_MSGTYPE_VIDEO,
    VIDEO_RTMP_MSGTYPE_10,
    VIDEO_RTMP_MSGTYPE_AMF3_CMD_ALT,
    VIDEO_RTMP_MSGTYPE_12,
    VIDEO_RTMP_MSGTYPE_13,
    VIDEO_RTMP_MSGTYPE_14,
    VIDEO_RTMP_MSGTYPE_AMF3_META,
    VIDEO_RTMP_MSGTYPE_16,
    VIDEO_RTMP_MSGTYPE_AMF3_CMD,
    VIDEO_RTMP_MSGTYPE_AMF0_META,
    VIDEO_RTMP_MSGTYPE_19,
    VIDEO_RTMP_MSGTYPE_AMF0_CMD
};
static char *video_rtmp_msgtype_names[] = {
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

typedef enum video_rtmp_state_e video_rtmp_state;
enum video_rtmp_state_e {
    VIDEO_RTMP_STATE_S0,
    VIDEO_RTMP_STATE_S1,
    VIDEO_RTMP_STATE_S2,
    VIDEO_RTMP_STATE_HEADER,
    VIDEO_RTMP_STATE_BODY
};

typedef enum video_rtmp_kind_e video_rtmp_kind;
enum video_rtmp_kind_e {
    VIDEO_RTMP_KIND_UNDECIDED,
    VIDEO_RTMP_KIND_PUBISHER,
    VIDEO_RTMP_KIND_SUBSCRIBER
};

typedef struct video_rtmp_chunk_s video_rtmp_chunk_t;
struct video_rtmp_chunk_s {
    uint32_t chunkid;

    uint32_t timestamp;
    uint32_t timediff;
    uint32_t length;
    video_rtmp_msgtype type;
    uint32_t streamid;

    uv_buf_t msgbodybuf;
};

typedef struct video_rtmp_local_s video_rtmp_local_t;
struct video_rtmp_local_s {
    video_thread_t *thread;
    uv_tcp_t client;
    video_rtmp_state state;
    video_rtmp_kind kind;
    video_channel_publisher_t publisher;
    video_channel_subscriber_t subscriber;

    video_list_t chunks;
    video_rtmp_chunk_t *header;

    uint32_t inchunksiz;
    uint32_t outchunksiz;

    uv_buf_t handshakebuf;
    uv_buf_t msgheaderbuf;
};

void video_rtmp_entry(video_thread_t *thread);
void video_rtmp_async(video_thread_t *thread, video_async_t *async);
void video_rtmp_exit(video_thread_t *thread);

void video_rtmp_disconnect(video_rtmp_local_t *local);
void *video_rtmp_alloc(video_rtmp_local_t *local, size_t size);
void video_rtmp_write(video_rtmp_local_t *local, uv_buf_t *buf);

char *video_rtmp_handshake_s0(video_rtmp_local_t *local, char *ptr, size_t len);
char *video_rtmp_handshake_s1(video_rtmp_local_t *local, char *ptr, size_t len);
char *video_rtmp_handshake_s2(video_rtmp_local_t *local, char *ptr, size_t len);
char *video_rtmp_header(video_rtmp_local_t *local, char *ptr, size_t len);
char *video_rtmp_body(video_rtmp_local_t *local, char *ptr, size_t len);

#endif //VIDEO_RTMP_H
