
add_executable(client
   main.cpp
)

target_link_libraries(client m)
target_link_libraries(client pthread)
target_link_libraries(client ${Boost_LIBRARIES})
