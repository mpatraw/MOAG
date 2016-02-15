
cmake_minimum_required(VERSION 2.6)
project(moag)

set(CMAKE_CXX_COMPILER clang++)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

set(SOURCE
    src/serialize.cpp
    src/message.cpp
    src/encoding.cpp
    src/network.cpp
    src/server.cpp
    src/client.cpp
)

set(TEST
    test/main.cpp
)

set(CMAKE_CXX_FLAGS "-g -std=c++14 -Wall -pedantic")

install_files(/bin FILES bin/moag)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_executable(moag ${SOURCE} src/main.cpp)
target_link_libraries(moag snappy enet SDL2 SDL2_ttf SDL2_image pthread)

#add_executable(test ${SOURCE} test/main.cpp)
#target_link_libraries(test snappy enet SDL2 SDL2_ttf SDL2_image pthread)
