
cmake_minimum_required(VERSION 2.6)
project(moag)

set(CMAKE_CXX_COMPILER clang++)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

set(SOURCE
    src/serialize.cpp
    src/encoding.cpp
    src/network.cpp
    src/server.cpp
    src/client.cpp
    src/main.cpp
)

set(CMAKE_CXX_FLAGS "-g -std=c++14 -Wall -pedantic")

install_files(/bin FILES bin/moag)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_executable(moag ${SOURCE})
target_link_libraries(moag RakNetDLL enet SDL2 SDL2_ttf SDL2_image pthread)

