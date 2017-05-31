cmake_minimum_required(VERSION 3.6)
project(newclient)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES client.cpp)
add_executable(newclient ${SOURCE_FILES})