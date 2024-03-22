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

add_library(${PROJECT_NAME} STATIC
        new-seika/input.c
        new-seika/seika.c
        new-seika/platform.c
        new-seika/string.c
        new-seika/math.c
        new-seika/memory.c

#        seika/seika.c
#        seika/asset/asset_manager.c
#        seika/asset/asset_file_loader.c
#        seika/memory/se_mem.c
#        seika/math/se_curve_float.c
#        seika/math/se_math.c
#        seika/input/input.c
#        seika/input/input_action.c
#        seika/input/mouse.c
#        seika/networking/se_network.c
#        seika/networking/se_network_socket.c
#        seika/rendering/font.c
#        seika/rendering/frame_buffer.c
#        seika/rendering/renderer.c
#        seika/rendering/render_context.c
#        seika/rendering/shader/shader.c
#        seika/rendering/shader/shader_cache.c
#        seika/rendering/shader/shader_file_parser.c
#        seika/rendering/shader/shader_instance.c
#        seika/rendering/texture.c
#        seika/audio/audio_manager.c
#        seika/audio/audio.c
#        seika/thread/se_pthread.c
#        seika/thread/se_thread_pool.c
#        seika/utils/command_line_args_util.c
#        seika/utils/logger.c
#        seika/utils/se_file_system_utils.c
#        seika/utils/se_platform.c
#        seika/utils/se_string_util.c
#        seika/utils/observer.c
#        seika/data_structures/ska_array2d.c
#        seika/data_structures/ska_linked_list.c
#        seika/data_structures/se_array_utils.c
#        seika/data_structures/ska_array_list.c
#        seika/data_structures/se_queue.c
#        seika/data_structures/se_hash_map.c
#        seika/data_structures/se_hash_map_string.c
#        seika/data_structures/se_spatial_hash_map.c
#        seika/data_structures/se_tile_map.c
#        seika/ecs/ecs.c
#        seika/ecs/component.c
#        seika/ecs/entity.c
#        seika/ecs/ec_system.c
        )

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

#--- Dependencies ---#
include(Dependencies.cmake)

#--- Link ---#
if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
#    target_link_libraries(${PROJECT_NAME} PUBLIC glad stb_image zip SDL2::SDL2main SDL2::SDL2 freetype Ws2_32)
    target_link_libraries(${PROJECT_NAME} PUBLIC cglm glad stb_image SDL3::SDL3-static freetype Ws2_32)
elseif (WIN32)
#    target_link_libraries(${PROJECT_NAME} PUBLIC glad stb_image zip -lmingw32 -static-libgcc SDL2::SDL2main SDL2::SDL2 freetype -lws2_32)
    target_link_libraries(${PROJECT_NAME} PUBLIC cglm glad stb_image -lmingw32 -static-libgcc SDL3::SDL3-static freetype -lws2_32)
elseif (APPLE)
#    target_link_libraries(${PROJECT_NAME} PUBLIC glad stb_image zip -Xlinker SDL2::SDL2main SDL2::SDL2 freetype m)
    target_link_libraries(${PROJECT_NAME} PUBLIC glad stb_image zip -Xlinker SDL2::SDL2main SDL2::SDL2 freetype m)
else ()
#    target_link_libraries(${PROJECT_NAME} PUBLIC glad stb_image zip -static-libgcc -Xlinker -export-dynamic SDL2::SDL2main SDL2::SDL2 freetype m)
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
    # Test main
    add_executable(${PROJECT_NAME}_app new-seika/main.c)
    target_link_libraries(${PROJECT_NAME}_app ${PROJECT_NAME})

    # Create seika test exe
#    add_executable(${PROJECT_NAME}_test test/main.c)
#    target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME} unity)
endif()
