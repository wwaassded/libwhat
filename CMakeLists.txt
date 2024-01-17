cmake_minimum_required(VERSION 3.22)
project(libwhat)

set(CMAKE_CXX_STANDARD 17)

add_executable(libwhat src/core/buffer.cc src/core/socket.cc src/core/thread_pool.cc src/core/net_address.cc src/include/socket.h src/include/net_address.h src/include/thread_pool.h src/include/buffer.h  main.cc)