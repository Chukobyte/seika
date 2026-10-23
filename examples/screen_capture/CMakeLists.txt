cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

set(CMAKE_C_STANDARD 11)

project(seika_example_screen_capture C)

if (WIN32)
    add_executable(${PROJECT_NAME} WIN32 main.c)
else ()
    add_executable(${PROJECT_NAME} main.c)
endif ()
target_link_libraries(${PROJECT_NAME} PRIVATE seika)
