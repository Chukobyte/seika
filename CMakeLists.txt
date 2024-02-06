cmake_minimum_required(VERSION 3.25.0)

set(CMAKE_C_STANDARD 11)

project(seika C)

if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
#    list(APPEND flags "/std:c11" "/W3" "/Zc:preprocessor") # TODO: Adding "/std:c11" breaks editor, fix later...
    list(APPEND flags "/W3" "/Zc:preprocessor")
elseif (APPLE)
    list(APPEND flags "-Wfatal-errors" "-Wall" "-Wextra" "-Wno-write-strings" "-Wno-deprecated-declarations"
            "-Wno-unused-variable" "-Wno-bad-function-cast" "-Wno-unused-parameter" "-Wno-missing-field-initializers")
else ()
    list(APPEND flags "-std=c11" "-Wfatal-errors" "-Wall" "-Wextra" "-Wno-write-strings" "-Wno-deprecated-declarations"
            "-Wno-unused-variable" "-Wno-cast-function-type" "-Wno-unused-parameter" "-Wno-missing-field-initializers")
endif ()

add_library(${PROJECT_NAME} STATIC
        seika/seika.c
        seika/asset/asset_manager.c
        seika/asset/asset_file_loader.c
        seika/memory/se_mem.c
        seika/math/se_curve_float.c
        seika/math/se_math.c
        seika/input/input.c
        seika/input/input_action.c
        seika/input/mouse.c
        seika/networking/se_network.c
        seika/networking/se_network_socket.c
        seika/rendering/font.c
        seika/rendering/frame_buffer.c
        seika/rendering/renderer.c
        seika/rendering/render_context.c
        seika/rendering/shader/shader.c
        seika/rendering/shader/shader_cache.c
        seika/rendering/shader/shader_file_parser.c
        seika/rendering/shader/shader_instance.c
        seika/rendering/texture.c
        seika/audio/audio_manager.c
        seika/audio/audio.c
        seika/thread/se_pthread.c
        seika/thread/se_thread_pool.c
        seika/utils/command_line_args_util.c
        seika/utils/logger.c
        seika/utils/se_file_system_utils.c
        seika/utils/se_platform.c
        seika/utils/se_string_util.c
        seika/utils/observer.c
        seika/data_structures/se_array_utils.c
        seika/data_structures/se_array_list.c
        seika/data_structures/se_queue.c
        seika/data_structures/se_hash_map.c
        seika/data_structures/se_hash_map_string.c
        seika/data_structures/se_spatial_hash_map.c
        seika/data_structures/se_tile_map.c
        seika/ecs/ecs.c
        seika/ecs/component.c
        seika/ecs/entity.c
        seika/ecs/ec_system.c
        )

#--- Dependencies ---#
find_package(SDL2 CONFIG REQUIRED)

find_package(freetype CONFIG)
set(freetype_from_find_package TRUE)
if (NOT freetype_FOUND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(freetype REQUIRED freetype2)
    set(freetype_from_find_package FALSE)
endif()

if (NOT TARGET glad)
    add_library(glad include/glad/glad.c)
endif()
if (NOT TARGET stb_image)
    add_library(stb_image include/stb_image/stb_image.c)
endif()
if (NOT TARGET kuba_zip)
    include(FetchContent)
    FetchContent_Declare(
            kuba_zip
            GIT_REPOSITORY https://github.com/kuba--/zip.git
            GIT_TAG v0.3.0
    )
    FetchContent_MakeAvailable(kuba_zip)
endif()
if (NOT TARGET unity)
    add_library(unity include/unity.c)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
        "${PROJECT_BINARY_DIR}"
        "include"
        "${PROJECT_SOURCE_DIR}"
        )
target_include_directories(glad PUBLIC
        "${PROJECT_BINARY_DIR}"
        "include"
        )
target_include_directories(stb_image PUBLIC
        "${PROJECT_BINARY_DIR}"
        "include"
        )

target_include_directories(unity PUBLIC
        "${PROJECT_BINARY_DIR}"
        "include"
        )

if (NOT freetype_from_find_package)
    target_include_directories(${PROJECT_NAME} PUBLIC ${freetype_INCLUDE_DIRS})
endif()

#--- Link ---#
if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_link_libraries(${PROJECT_NAME} PUBLIC glad stb_image zip SDL2::SDL2main SDL2::SDL2 freetype Ws2_32)
elseif (WIN32)
    target_link_libraries(${PROJECT_NAME} PUBLIC glad stb_image zip -lmingw32 -static-libgcc SDL2::SDL2main SDL2::SDL2 freetype -lws2_32)
elseif (APPLE)
    target_link_libraries(${PROJECT_NAME} PUBLIC glad stb_image zip -Xlinker SDL2::SDL2main SDL2::SDL2 freetype m)
else ()
    target_link_libraries(${PROJECT_NAME} PUBLIC glad stb_image zip -static-libgcc -Xlinker -export-dynamic SDL2::SDL2main SDL2::SDL2 freetype m)
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
    # Create seika test exe
    add_executable(${PROJECT_NAME}_test test/main.c)
    target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME} unity)
endif()
