cmake_minimum_required(VERSION 3.6)
project(video)

set(CMAKE_C_STANDARD 99)

macro(depurl_download name url)
    if (NOT EXISTS "libs/dist/${name}.tar.gz")
        file(DOWNLOAD "${url}" "libs/dist/${name}.tar.gz")
    endif()

    add_custom_command(
            OUTPUT "${CMAKE_BINARY_DIR}/libs/${name}"
            COMMAND mkdir -p "${CMAKE_BINARY_DIR}/libs/${name}"
            COMMAND tar -xvf "${CMAKE_BINARY_DIR}/libs/dist/${name}.tar.gz" -C "${CMAKE_BINARY_DIR}/libs/${name}" --strip-components=1
            DEPENDS "${CMAKE_BINARY_DIR}/libs/dist/${name}.tar.gz"
    )
    add_custom_target(${name}-dist DEPENDS "${CMAKE_BINARY_DIR}/libs/${name}")
endmacro()

depurl_download(libu "http://www.koanlogic.com/download/libu/libu-2.2.0.tar.gz")
depurl_download(makl "http://www.koanlogic.com/download/makl/makl-1.9.0.tar.gz")

add_custom_command(
        OUTPUT "${CMAKE_BINARY_DIR}/libs/makl/out"
        COMMAND make toolchain
        COMMAND ./configure --gnu_make=make --prefix="${CMAKE_BINARY_DIR}/libs/makl/out"
        COMMAND make install
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/libs/makl"
        DEPENDS makl-dist
)

add_custom_target(makl DEPENDS makl-dist "${CMAKE_BINARY_DIR}/libs/makl/out")


add_custom_command(
        OUTPUT "${CMAKE_BINARY_DIR}/libs/libu/out"
        COMMAND "${CMAKE_BINARY_DIR}/libs/makl/out/bin/makl-conf" --prefix="${CMAKE_BINARY_DIR}/libs/libu/out" --no_net
        COMMAND "${CMAKE_BINARY_DIR}/libs/makl/out/bin/makl"
        COMMAND "${CMAKE_BINARY_DIR}/libs/makl/out/bin/makl" install
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/libs/libu"
        DEPENDS libu-dist makl-dist
)
add_custom_target(libu DEPENDS makl "${CMAKE_BINARY_DIR}/libs/libu/out")

set(SOURCE_FILES main.c rtmp.c buffer.c)
add_executable(video ${SOURCE_FILES})
target_link_libraries(video uv "${CMAKE_BINARY_DIR}/libs/libu/out/lib/libu.a")
add_dependencies(video libu)