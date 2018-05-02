add_executable(server
   session.cpp
   server.cpp
   main.cpp
)

target_link_libraries(server m)
target_link_libraries(server pthread)
target_link_libraries(server ${Boost_LIBRARIES})
