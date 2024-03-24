cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

project(seika C)

if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    list(APPEND flags "/W3" "/Zc:preprocessor")
elseif (APPLE)
    list(APPEND flags "-Wfatal-errors" "-Wall" "-Wextra" "-Wno-write-strings" "-Wno-deprecated-declarations"
            "-Wno-unused-variable" "-Wno-bad-function-cast" "-Wno-unused-parameter" "-Wno-missing-field-initializers")
else ()
    list(APPEND flags "-std=c11" "-Wfatal-errors" "-Wall" "-Wextra" "-Wno-write-strings" "-Wno-deprecated-declarations"
            "-Wno-unused-variable" "-Wno-cast-function-type" "-Wno-unused-parameter" "-Wno-missing-field-initializers")
endif ()

file(GLOB_RECURSE SEIKA_SRC seika/*.c)
add_library(${PROJECT_NAME} STATIC ${SEIKA_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#--- Dependencies ---#
include(Dependencies.cmake)

#--- Link ---#
if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
#    target_link_libraries(${PROJECT_NAME} PUBLIC glad stb_image zip SDL2::SDL2main SDL2::SDL2 freetype Ws2_32)
    target_link_libraries(${PROJECT_NAME} PUBLIC cglm glad stb_image zip SDL3::SDL3-static freetype Ws2_32)
elseif (WIN32)
#    target_link_libraries(${PROJECT_NAME} PUBLIC glad stb_image zip -lmingw32 -static-libgcc SDL2::SDL2main SDL2::SDL2 freetype -lws2_32)
    target_link_libraries(${PROJECT_NAME} PUBLIC cglm glad stb_image zip -lmingw32 -static-libgcc SDL3::SDL3-static freetype -lws2_32)
elseif (APPLE)
#    target_link_libraries(${PROJECT_NAME} PUBLIC glad stb_image zip -Xlinker SDL2::SDL2main SDL2::SDL2 freetype m)
    target_link_libraries(${PROJECT_NAME} PUBLIC cglm glad stb_image zip -Xlinker SDL3::SDL3-static freetype m)
else ()
#    target_link_libraries(${PROJECT_NAME} PUBLIC glad stb_image zip -static-libgcc -Xlinker -export-dynamic SDL2::SDL2main SDL2::SDL2 freetype m)
    target_link_libraries(${PROJECT_NAME} PUBLIC cglm glad stb_image zip -static-libgcc -Xlinker -export-dynamic SDL3::SDL3-static freetype m)
endif ()

target_compile_options(${PROJECT_NAME} PUBLIC ${flags})

# Copy directories over that are needed to test
if (NOT DEFINED IS_CI_BUILD)
    set(IS_CI_BUILD "false")
endif()
if (IS_CI_BUILD STREQUAL "false")
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
    # Temp testing with main, move/update to seika-example repo later
    add_executable(${PROJECT_NAME}_app seika/main.c)
    target_link_libraries(${PROJECT_NAME}_app ${PROJECT_NAME})

    # Create seika test exe
    add_executable(${PROJECT_NAME}_test test/test.c)
    target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME} unity)
endif()
