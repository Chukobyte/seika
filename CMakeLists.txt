cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_C_STANDARD 11)

project(seika-framework C)

option(IS_CI_BUILD "" OFF)
option(SEIKA_STATIC_LIB "Make seika and dependent libs static" OFF)

# Optional Modules
option(SEIKA_ECS "Will enable ecs" ON)
option(SEIKA_INPUT "Will enable input" ON)
option(SEIKA_RENDERING "Will enable rendering" ON)
option(SEIKA_AUDIO "Will enable audio" ON)
option(SEIKA_NETWORK "Will enable networking" ON)
option(SEIKA_SDL "Will enable sdl" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

#--- Dependencies ---#
include(Dependencies.cmake)

add_subdirectory(seika)
target_compile_definitions(seika
        PUBLIC
        SKA_ECS=$<IF:$<BOOL:${SEIKA_ECS}>,1,0>
        SKA_INPUT=$<IF:$<BOOL:${SEIKA_INPUT}>,1,0>
        SKA_RENDERING=$<IF:$<BOOL:${SEIKA_RENDERING}>,1,0>
        SKA_AUDIO=$<IF:$<BOOL:${SEIKA_AUDIO}>,1,0>
        SKA_NETWORK=$<IF:$<BOOL:${SEIKA_NETWORK}>,1,0>
        SKA_SDL=$<IF:$<BOOL:${SEIKA_SDL}>,1,0>
)

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    set(SK_IS_MAIN TRUE)
else()
    set(SK_IS_MAIN FALSE)
endif()

if (SK_IS_MAIN)
    # Create seika test exe
    add_executable(seika_test test/test.c)
    target_link_libraries(seika_test seika unity)
    if (NOT IS_CI_BUILD)
        # Copy directories over that are needed to test
        add_custom_command(TARGET seika_test POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_SOURCE_DIR}/test
                $<TARGET_FILE_DIR:seika_test>/test
        )
        if (SEIKA_INPUT OR SEIKA_RENDERING)
            # Create test game executable
            add_executable(seika_test_game test/game/src/main.c)
            target_link_libraries(seika_test_game seika)
        endif ()
    endif()
endif()
