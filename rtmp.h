//
// Created by user on 2/12/17.
//

#ifndef VIDEO_RTMP_H
#define VIDEO_RTMP_H

#include <uv.h>

void rtmp_init();
void rtmp_connected(uv_stream_t* stream);

#endif //VIDEO_RTMP_H
