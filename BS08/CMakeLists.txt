cmake_minimum_required(VERSION 3.15)
project(BS08 C)

set(CMAKE_C_STANDARD 11)

add_executable(BS08 main.c include/ustarFunctions.h lib/ustarFunctions.c)