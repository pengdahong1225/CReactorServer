cmake_minimum_required(VERSION 3.24)

set(TARGET core)
set(CMAKE_CXX_STANDARD 20)

if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
endif ()

project(${TARGET})

include_directories(core/Common)
include_directories(../libs)

aux_source_directory(core SRCS)
aux_source_directory(core/Common SRCS)
aux_source_directory(core/Log SRCS)
aux_source_directory(core/Net SRCS)
aux_source_directory(core/Net/Poller SRCS)
aux_source_directory(core/TcpServer SRCS)
