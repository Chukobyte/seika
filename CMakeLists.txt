cmake_minimum_required(VERSION 3.27.0)

project(seika-framework C)

set(CMAKE_C_STANDARD 11)

if (NOT DEFINED SEIKA_UNIT_TESTS)
    option(SEIKA_UNIT_TESTS "Will enable seika unit tests" ON)
endif ()

if (SEIKA_UNIT_TESTS)
    enable_testing()
endif ()

add_subdirectory(seika)
