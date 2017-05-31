cmake_minimum_required(VERSION 3.6)
project(server2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp
        roadMap.cpp
        roadMap.h)
add_executable(server2 ${SOURCE_FILES})