cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_C_STANDARD 11)

project(seika1 C)

if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    list(APPEND flags "/W3" "/Zc:preprocessor")
elseif (APPLE)
    list(APPEND flags "-Wfatal-errors" "-Wall" "-Wextra" "-Wno-write-strings" "-Wno-deprecated-declarations"
            "-Wno-unused-variable" "-Wno-bad-function-cast" "-Wno-unused-parameter" "-Wno-missing-field-initializers")
else ()
    list(APPEND flags "-std=c11" "-Wfatal-errors" "-Wall" "-Wextra" "-Wno-write-strings" "-Wno-deprecated-declarations"
            "-Wno-unused-variable" "-Wno-cast-function-type" "-Wno-unused-parameter" "-Wno-missing-field-initializers")
endif ()

file(GLOB_RECURSE SEIKA_SRC ./*.c ./*.h)
add_library(${PROJECT_NAME} STATIC ${SEIKA_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)

set(LIBS_TO_LINK "")

if (SEIKA_ENABLE_RENDERING)
    list(APPEND LIBS_TO_LINK glad stb_image freetype)
endif()

if (SEIKA_ENABLE_AUDIO)
    list(APPEND LIBS_TO_LINK miniaudio)
endif()

if (SEIKA_ENABLE_INPUT)
    list(APPEND LIBS_TO_LINK ${SDL3_LIBRARY})
endif()

#--- Link ---#
if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    list(APPEND LIBS_TO_LINK Ws2_32)
elseif (WIN32)
    list(APPEND LIBS_TO_LINK -lmingw32 -static-libgcc -lws2_32)
elseif (APPLE)
    list(APPEND LIBS_TO_LINK m)
else ()
    list(APPEND LIBS_TO_LINK -static-libgcc -Xlinker -export-dynamic m)
endif ()

target_link_libraries(${PROJECT_NAME} PUBLIC cglm zip ${LIBS_TO_LINK})

target_compile_options(${PROJECT_NAME} PUBLIC ${flags})
