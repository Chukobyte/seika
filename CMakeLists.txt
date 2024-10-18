cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_C_STANDARD 11)

project(seika-framework C)

option(IS_CI_BUILD "" OFF)
option(SEIKA_STATIC_LIB "Make seika and dependent libs static" OFF)
option(SEIKA_ENABLE_OLD "Will build old seika lib" OFF)
option(SEIKA_ENABLE_1 "Will build new seika lib" ON)

# Compilation Modes
option(SEIKA_ENABLE_ECS "Will enable ecs" ON)
option(SEIKA_ENABLE_INPUT "Will enable input" OFF)
option(SEIKA_ENABLE_RENDERING "Will enable rendering" OFF)
option(SEIKA_ENABLE_AUDIO "Will enable audio" OFF)

add_compile_definitions(SKA_ECS=$<IF:$<BOOL:${SEIKA_ENABLE_ECS}>,1,0>)
add_compile_definitions(SKA_INPUT=$<IF:$<BOOL:${SEIKA_ENABLE_INPUT}>,1,0>)
add_compile_definitions(SKA_RENDERING=$<IF:$<BOOL:${SEIKA_ENABLE_RENDERING}>,1,0>)
add_compile_definitions(SKA_AUDIO=$<IF:$<BOOL:${SEIKA_ENABLE_AUDIO}>,1,0>)

function(set_common_compile_definitions target)
    target_compile_definitions(${target} PRIVATE SKA_ECS=$<IF:$<BOOL:${SEIKA_ENABLE_ECS}>,1,0>)
    target_compile_definitions(${target} PRIVATE SKA_INPUT=$<IF:$<BOOL:${SEIKA_ENABLE_INPUT}>,1,0>)
    target_compile_definitions(${target} PRIVATE SKA_RENDERING=$<IF:$<BOOL:${SEIKA_ENABLE_RENDERING}>,1,0>)
    target_compile_definitions(${target} PRIVATE SKA_AUDIO=$<IF:$<BOOL:${SEIKA_ENABLE_AUDIO}>,1,0>)
endfunction()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

#--- Dependencies ---#
include(Dependencies.cmake)

if (SEIKA_ENABLE_OLD)
    add_subdirectory(seika)
endif()
if (SEIKA_ENABLE_1)
    add_subdirectory(seika1)
endif()

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    set(SK_IS_MAIN TRUE)
else()
    set(SK_IS_MAIN FALSE)
endif()

if (SK_IS_MAIN)
    if (SEIKA_ENABLE_OLD)
        # Create seika test exe
        add_executable(seika_test test/test.c)
        target_link_libraries(seika_test seika unity)
    endif()
    if (SEIKA_ENABLE_1)
        # Create seika1 test exe
        add_executable(seika1_test test/test1.c)
        target_link_libraries(seika1_test seika1 unity)
        set_common_compile_definitions(seika1_test)
    endif()
endif()
