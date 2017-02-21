//
// Created by user on 2/18/17.
//

#ifndef VIDEO_MP4_H
#define VIDEO_MP4_H

#include <uv.h>
#include "global/global.h"

void video_mp4_entry(video_thread_t *thread);
void video_mp4_async(video_thread_t *thread, video_async_t *async);
void video_mp4_exit(video_thread_t *thread);

#endif //VIDEO_MP4_H
