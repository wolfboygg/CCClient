cmake_minimum_required(VERSION 3.17)
project(CStreamClient)

set(CMAKE_CXX_STANDARD 11)

add_executable(CStreamClient main.cpp CStreamClient.cpp CThread.cpp)