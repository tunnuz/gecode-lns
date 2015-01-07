cmake_minimum_required(VERSION 2.8)

project(GECODELNS)
set(CMAKE_CXX_FLAGS "-std=c++11 -O3")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${GECODELNS_SOURCE_DIR}/cmake) 

find_package(Gecode REQUIRED)
include_directories(${GECODE_INCLUDE_DIRS})
set(LIBS ${LIBS} ${GECODE_LIBRARIES})

include_directories(${GECODELNS_SOURCE_DIR}/include)

add_subdirectory(src)
add_subdirectory(test)
