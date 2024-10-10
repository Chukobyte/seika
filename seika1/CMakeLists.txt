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
