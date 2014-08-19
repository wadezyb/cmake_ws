cmake_minimum_required(VERSION 2.8)
project(myproject)

set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/inc)

set(PROJECT1_SRCS 
${PROJECT_SOURCE_DIR}/test.cpp
)

include_directories( "${PROJECT_INCLUDE_DIR}" )
include_directories( "${PROJECT_SOURCE_DIR}" )

add_executable(test ${PROJECT1_SRCS})
#add_subdirectory(src)
