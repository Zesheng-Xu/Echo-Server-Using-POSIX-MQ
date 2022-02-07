cmake_minimum_required(VERSION 3.21)
project(Programming1)

set(CMAKE_CXX_STANDARD 14)

add_executable(Programming1 program1.cpp)
find_library(LIBRT rt)
if(LIBRT)
    target_link_libraries(Programming1 ${LIBRT})
endif()