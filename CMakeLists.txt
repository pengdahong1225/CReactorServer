cmake_minimum_required(VERSION 3.23)

set(TARGET net)
set(CMAKE_CXX_STANDARD 20)

if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
endif ()

#add_definitions(-D_POLL)

project(${TARGET})

include_directories(.)
include_directories(./Common)
aux_source_directory(. SRCS)
aux_source_directory(./Common SRCS)
aux_source_directory(./Interface SRCS)
aux_source_directory(./Net SRCS)

add_library(${TARGET} STATIC ${SRCS})
