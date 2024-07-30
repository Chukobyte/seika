cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_C_STANDARD 11)

project(seika C)

option(IS_CI_BUILD "" OFF)
option(SEIKA_STATIC_LIB "Make seika and dependent libs static" OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    list(APPEND flags "/W3" "/Zc:preprocessor")
elseif (APPLE)
    list(APPEND flags "-Wfatal-errors" "-Wall" "-Wextra" "-Wno-write-strings" "-Wno-deprecated-declarations"
            "-Wno-unused-variable" "-Wno-bad-function-cast" "-Wno-unused-parameter" "-Wno-missing-field-initializers")
else ()
    list(APPEND flags "-std=c11" "-Wfatal-errors" "-Wall" "-Wextra" "-Wno-write-strings" "-Wno-deprecated-declarations"
            "-Wno-unused-variable" "-Wno-cast-function-type" "-Wno-unused-parameter" "-Wno-missing-field-initializers")
endif ()

file(GLOB_RECURSE SEIKA_SRC seika/*.c seika/*.h)
add_library(${PROJECT_NAME} STATIC ${SEIKA_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#--- Dependencies ---#
include(Dependencies.cmake)

#--- Link ---#
if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_link_libraries(${PROJECT_NAME} PUBLIC cglm glad stb_image zip miniaudio ${SDL3_LIBRARY} freetype Ws2_32)
elseif (WIN32)
    target_link_libraries(${PROJECT_NAME} PUBLIC cglm glad stb_image zip miniaudio -lmingw32 -static-libgcc ${SDL3_LIBRARY} freetype -lws2_32)
elseif (APPLE)
    target_link_libraries(${PROJECT_NAME} PUBLIC cglm glad stb_image zip miniaudio -Xlinker ${SDL3_LIBRARY} freetype m)
else ()
    target_link_libraries(${PROJECT_NAME} PUBLIC cglm glad stb_image zip miniaudio -static-libgcc -Xlinker -export-dynamic ${SDL3_LIBRARY} freetype m)
endif ()

target_compile_options(${PROJECT_NAME} PUBLIC ${flags})

# Copy directories over that are needed to test
if (NOT IS_CI_BUILD)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${PROJECT_SOURCE_DIR}/test
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/test
            )
endif()

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    set(SK_IS_MAIN TRUE)
else()
    set(SK_IS_MAIN FALSE)
endif()

if (SK_IS_MAIN)
    # Create seika test exe
    add_executable(${PROJECT_NAME}_test test/test.c)
    target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME} unity)
endif()
