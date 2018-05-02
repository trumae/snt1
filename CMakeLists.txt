project(snt1)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -pedantic -Wextra")
find_package( Boost 1.52 COMPONENTS system thread filesystem REQUIRED )

#include_directories( ${Boost_INCLUDE_DIR} )

add_subdirectory(client)
add_subdirectory(server)
