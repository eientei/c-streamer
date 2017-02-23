//
// Created by user on 2/21/17.
//

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "channel.h"

static void video_channel_deliver(uv_async_t *async) {
    video_channel_subscriber_t *subscriber = async->data;

    //size_t i=0;
    while (1) {
        //i++;
        video_message_t *message = video_queue_dequeue(&subscriber->queue);
        if (!message) {
            break;
        }
        //printf("%ld\n", message->len);
        subscriber->update(message, subscriber->data);
        video_slab_unref(message);
    }
    //printf("> %ld\n", i);
}

void video_channel_init(video_channel_t *channel, char *name) {
    uv_mutex_init(&channel->mutex);
    channel->name = strdup(name);
    video_list_init(&channel->subscribers);
}

void video_channel_deinit(video_channel_t *channel) {
    free(channel->name);
    video_list_deinit(&channel->subscribers);
    uv_mutex_destroy(&channel->mutex);
    free(channel);
}

void video_channel_publish(video_channel_publisher_t *publisher, video_channel_t *channel, video_thread_t *thread) {
    uv_mutex_lock(&channel->mutex);
    publisher->channel = channel;
    publisher->since = time(NULL);
    publisher->state = VIDEO_CHANNEL_PUBLISHER_STATE_ACTIVE;
    publisher->thread = thread;
    uv_mutex_unlock(&channel->mutex);
}

void video_channel_unpublish(video_channel_publisher_t *publisher) {
    uv_mutex_lock(&publisher->channel->mutex);
    uv_mutex_unlock(&publisher->channel->mutex);
}

void video_channel_subscribe(video_channel_subscriber_t* subscriber, video_channel_t *channel, video_thread_t *thread, video_channel_update_cb update, void *data) {
    uv_mutex_lock(&channel->mutex);
    subscriber->channel = channel;
    subscriber->data = data;
    subscriber->update = update;
    video_queue_init(&subscriber->queue);
    subscriber->latch.data = subscriber;
    uv_async_init(&thread->loop, &subscriber->latch, video_channel_deliver);
    uv_mutex_unlock(&channel->mutex);
}

void video_channel_unsubscribe(video_channel_subscriber_t *subscriber) {
    uv_mutex_lock(&subscriber->channel->mutex);
    video_queue_deinit(&subscriber->queue);
    uv_mutex_unlock(&subscriber->channel->mutex);
    uv_close((uv_handle_t *) &subscriber->latch, 0);
}

void video_channel_broadcast(video_channel_publisher_t *publisher, video_message_type type, uint32_t  time, void *data, size_t len) {
    uv_mutex_lock(&publisher->channel->mutex);
    video_message_t *message = video_slab_alloc(&publisher->thread->slab, sizeof(video_message_t));
    message->channel = publisher->channel;
    message->type = type;
    message->data = data;
    message->len = len;
    message->time = time;
    for (size_t i = 0; i < publisher->channel->subscribers.size; i++) {
        video_channel_subscriber_t *subscriber = publisher->channel->subscribers.nodes[i].data;
        video_slab_ref(message->data);
        video_slab_ref(message);
        video_queue_enqueue(&subscriber->queue, message);
        uv_async_send(&subscriber->latch);
    }
    video_slab_unref(message);
    uv_mutex_unlock(&publisher->channel->mutex);
}