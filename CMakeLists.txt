cmake_minimum_required(VERSION 3.24)

set(TARGET net)
set(CMAKE_CXX_STANDARD 20)

if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
endif ()

#add_definitions(-D_POLL)

project(${TARGET})

include_directories(Common)
include_directories(Base)

aux_source_directory(. SRCS)
aux_source_directory(Common SRCS)
aux_source_directory(Base SRCS)
aux_source_directory(Net SRCS)
aux_source_directory(Net/Poller SRCS)

add_library(${TARGET} STATIC ${SRCS})
