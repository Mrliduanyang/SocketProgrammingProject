cmake_minimum_required(VERSION 3.10)
project(SocketProgrammingProject)

set(CMAKE_CXX_STANDARD 17)

add_executable(serverA serverA.cpp)
add_executable(serverB serverB.cpp)
add_executable(aws aws.cpp)
add_executable(client client.cpp)