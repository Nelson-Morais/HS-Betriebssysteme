cmake_minimum_required(VERSION 3.15)
project(BS06 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS -lm -lssl -lcrypto -pthread -lcurl)

add_executable(BS06 main.c)

