add_executable(tsp_lns tsp_lns.cc)

target_link_libraries(tsp_lns ${GECODE_LIBRARIES} gecode-lns)
