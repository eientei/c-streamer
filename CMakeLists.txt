cmake_minimum_required(VERSION 3.6)
project(video)

set(CMAKE_C_STANDARD 99)

include_directories(${CMAKE_SOURCE_DIR})
set(AMF_SOURCE_FILES amf/amf.c)
set(RTMP_SOURCE_FILES rtmp/rtmp.c rtmp/handshake.c rtmp/message.c)
set(MP4_SOURCE_FILES mp4/mp4.c)
set(GLOBAL_SOURCE_FILES global/global.c)
set(UTIL_SOURCE_FILES util/generic.c util/list.c util/buffer.c util/map.c util/queue.c)
set(SOURCE_FILES main.c ${UTIL_SOURCE_FILES} ${GLOBAL_SOURCE_FILES} ${AMF_SOURCE_FILES} ${RTMP_SOURCE_FILES} ${MP4_SOURCE_FILES})
add_executable(video ${SOURCE_FILES})
target_link_libraries(video uv crypto)