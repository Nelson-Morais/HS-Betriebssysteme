cmake_minimum_required(VERSION 3.15)

project(BS06 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS -pthread)
find_package(CURL REQUIRED)

include_directories(${CURL_INCLUDE_DIR})
add_executable(BS06 main.c)
target_link_libraries(BS06 ${CURL_LIBRARIES})