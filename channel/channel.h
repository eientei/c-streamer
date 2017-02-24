//
// Created by user on 2/21/17.
//

#ifndef VIDEO_CHANNEL_H
#define VIDEO_CHANNEL_H

#include <uv.h>
#include "util/list.h"
#include "util/queue.h"
#include "global/global.h"

typedef enum video_message_type_e video_message_type;
enum video_message_type_e {
    VIDEO_MESSAGE_TYPE_BEGIN,
    VIDEO_MESSAGE_TYPE_META,
    VIDEO_MESSAGE_TYPE_VIDEO,
    VIDEO_MESSAGE_TYPE_AUDIO,
    VIDEO_MESSAGE_TYPE_DONE
};

typedef struct video_message_s video_message_t;
struct video_message_s {
    struct video_channel_s *channel;
    video_message_type type;
    char *data;
    size_t len;
    uint32_t time;
};

typedef enum video_channel_publisher_state_e video_channel_publisher_state;
enum video_channel_publisher_state_e {
    VIDEO_CHANNEL_PUBLISHER_STATE_IDLE,
    VIDEO_CHANNEL_PUBLISHER_STATE_ACTIVE
};

typedef struct video_channel_publisher_s video_channel_publisher_t;
struct video_channel_publisher_s {
    struct video_channel_s *channel;
    video_channel_publisher_state state;
    time_t since;
    video_thread_t *thread;
};

typedef void (*video_channel_update_cb)(video_message_t *message, void *data);

typedef struct video_channel_subscriber_s video_channel_subscriber_t;
struct video_channel_subscriber_s {
    struct video_channel_s *channel;
    uv_async_t latch;
    video_queue_t queue;
    video_channel_update_cb update;
    void *data;
};

typedef struct video_channel_s video_channel_t;
struct video_channel_s {
    video_channel_publisher_t *publisher;
    video_list_t subscribers;
    uv_mutex_t mutex;
    char *name;
};

void video_channel_init(video_channel_t *channel, char *name);
void video_channel_deinit(video_channel_t *channel);
void video_channel_publish(video_channel_publisher_t *publisher, video_channel_t *channel, video_thread_t *thread);
void video_channel_unpublish(video_channel_publisher_t *publisher);
void video_channel_subscribe(video_channel_subscriber_t *subscriber, video_channel_t *channel, video_thread_t *thread, video_channel_update_cb update, void *data);
void video_channel_unsubscribe(video_channel_subscriber_t *subscriber);
void video_channel_broadcast(video_channel_publisher_t *publisher, video_message_type type, uint32_t time, void *data, size_t len);

#endif //VIDEO_CHANNEL_H
