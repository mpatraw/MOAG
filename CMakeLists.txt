
cmake_minimum_required(VERSION 2.6)
project(moag)

set(SOURCES
    src/common.c
    src/encoding.c
    src/sdl_aux.c
    src/server.c
    src/xor128.c
    src/client.c
    src/main.cpp
)

set(CMAKE_CXX_FLAGS "-g -std=c++14")

install_files(/bin FILES bin/moag)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_executable(moag ${SOURCES})
target_link_libraries(moag SDL SDL_ttf enet SDL2 pthread)
