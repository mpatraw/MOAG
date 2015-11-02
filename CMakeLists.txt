
cmake_minimum_required(VERSION 2.6)
project(moag)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

set(SOURCES
    src/common.c
    src/encoding.c
    src/xor128.c
    src/server.cpp
    src/client.cpp
    src/main.cpp
)

set(CMAKE_CXX_FLAGS "-g -std=c++14 -Wall")

install_files(/bin FILES bin/moag)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_executable(moag ${SOURCES})
target_link_libraries(moag enet SDL2 SDL2_ttf SDL2_image pthread)
