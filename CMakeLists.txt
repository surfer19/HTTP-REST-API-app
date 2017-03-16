cmake_minimum_required(VERSION 3.5)
project(http_app_cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


set(SERVER server.cpp)
set(CLIENT client.cpp )
add_executable(server ${SERVER})
add_executable(client ${CLIENT})