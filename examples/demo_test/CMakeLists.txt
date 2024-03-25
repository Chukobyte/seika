cmake_minimum_required(VERSION 3.27.0)
project(demo_test C)

set(CMAKE_C_STANDARD 11)

add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} seika)
