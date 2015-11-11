
cmake_minimum_required(VERSION 2.6)
project(moag)

set(CMAKE_CXX_COMPILER clang++)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

set(COMMON
    src/encoding.cpp
    src/network.cpp
)

set(MOAG
    src/server.cpp
    src/client.cpp
    src/main.cpp
)
set(TEST
    src/test.cpp
)

set(CMAKE_CXX_FLAGS "-g -std=c++14 -Wall -pedantic")

install_files(/bin FILES bin/moag)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_executable(moag ${COMMON} ${MOAG})
target_link_libraries(moag enet SDL2 SDL2_ttf SDL2_image pthread)

add_executable(test ${COMMON} ${TEST})
target_link_libraries(test enet SDL2 SDL2_ttf SDL2_image pthread)
