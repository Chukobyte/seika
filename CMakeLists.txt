cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_C_STANDARD 11)

project(seika-framework C)

option(IS_CI_BUILD "" OFF)
option(SEIKA_STATIC_LIB "Make seika and dependent libs static" OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

#--- Dependencies ---#
include(Dependencies.cmake)

add_subdirectory(seika)
add_subdirectory(seika1)

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    set(SK_IS_MAIN TRUE)
else()
    set(SK_IS_MAIN FALSE)
endif()

if (SK_IS_MAIN)
    # Create seika test exe
    add_executable(seika_test test/test.c)
    target_link_libraries(seika_test seika unity)
endif()
