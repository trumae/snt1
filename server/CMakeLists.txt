add_executable(server
   main.cpp
)

target_link_libraries(server m)
target_link_libraries(server pthread)
target_link_libraries(server ${Boost_LIBRARIES})
