cmake_minimum_required(VERSION 3.6)
project(video)

set(CMAKE_C_STANDARD 99)

include_directories(${CMAKE_SOURCE_DIR})
set(RTMP_SOURCE_FILES rtmp/rtmp.c)
set(UTIL_SOURCE_FILES util/list.c util/buffer.c)
set(SOURCE_FILES main.c ${RTMP_SOURCE_FILES} ${UTIL_SOURCE_FILES})
add_executable(video ${SOURCE_FILES})
target_link_libraries(video uv crypto)