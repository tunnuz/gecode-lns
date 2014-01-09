cmake_minimum_required(VERSION 2.8)

project(GECODELNS)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${GECODELNS_SOURCE_DIR}/cmake) 
find_package(Gecode REQUIRED)
include_directories(${GECODE_INCLUDE_DIRS})
set(LIBS ${LIBS} ${GECODE_LIBRARIES})

add_library(gecode-lns lns.C meta_lns.C)

add_executable(tsp_lns tsp_lns.C)

target_link_libraries(tsp_lns ${GECODE_LIBRARIES} gecode-lns) 