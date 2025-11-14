cmake_minimum_required(VERSION 3.10)

set(TARGET .)
project(${TARGET} CXX)

set(CMAKE_CXX_STANDARD 20)

#add_definitions(-D_POLL)

include_directories(../3rdparty/google)
include_directories(include)

aux_source_directory(src SRCS)

add_library(${TARGET} STATIC ${SRCS})
