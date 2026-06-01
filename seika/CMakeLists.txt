cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_C_STANDARD 11)

project(seika C)

# Optional Modules
option(SEIKA_ECS "Will enable ecs" ON)
option(SEIKA_INPUT "Will enable input" ON)
option(SEIKA_RENDERING "Will enable rendering" ON)
option(SEIKA_AUDIO "Will enable audio" ON)
option(SEIKA_THREAD "Will enable thread library" ON)
option(SEIKA_NETWORK "Will enable networking" OFF)
option(SEIKA_HTTP_SERVER "Enable built-in static HTTP server" ON)
option(SEIKA_MAIN "Enable SKA_MAIN macro to simplify the main function" ON)

if (NOT DEFINED SEIKA_UNIT_TESTS)
    option(SEIKA_UNIT_TESTS "Will enable seika unit tests" ON)
endif ()

if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    list(APPEND flags "/W3" "/Zc:preprocessor")
elseif (APPLE)
    list(APPEND flags "-Wfatal-errors" "-Wall" "-Wextra" "-Wno-write-strings" "-Wno-deprecated-declarations"
            "-Wno-unused-variable" "-Wno-bad-function-cast" "-Wno-unused-parameter" "-Wno-missing-field-initializers")
else ()
    if("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
        list(APPEND flags "-std=gnu11" "-Wfatal-errors" "-Wall" "-Wextra" "-Wno-write-strings" "-Wno-deprecated-declarations"
                "-Wno-unused-variable" "-Wno-cast-function-type" "-Wno-unused-parameter" "-Wno-missing-field-initializers")
    else()
        list(APPEND flags "-std=c11" "-Wfatal-errors" "-Wall" "-Wextra" "-Wno-write-strings" "-Wno-deprecated-declarations"
                "-Wno-unused-variable" "-Wno-cast-function-type" "-Wno-unused-parameter" "-Wno-missing-field-initializers")
    endif()
endif ()

file(GLOB_RECURSE SEIKA_SRC ./seika/*.c ./seika/*.h)
add_library(${PROJECT_NAME} STATIC ${SEIKA_SRC})
#add_library(${PROJECT_NAME} STATIC
#        math/math.h
#        math/math.c
#)
target_compile_definitions(${PROJECT_NAME}
        PUBLIC
        SKA_ECS=$<IF:$<BOOL:${SEIKA_ECS}>,1,0>
        SKA_INPUT=$<IF:$<BOOL:${SEIKA_INPUT}>,1,0>
        SKA_RENDERING=$<IF:$<BOOL:${SEIKA_RENDERING}>,1,0>
        SKA_AUDIO=$<IF:$<BOOL:${SEIKA_AUDIO}>,1,0>
        SKA_THREAD=$<IF:$<BOOL:${SEIKA_THREAD}>,1,0>
#        SKA_NETWORK=$<IF:$<BOOL:${SEIKA_NETWORK}>,1,0>
        SKA_NETWORK=0
        SKA_HTTP_SERVER=$<IF:$<BOOL:${SEIKA_HTTP_SERVER}>,1,0>
        SKA_USE_MAIN=$<IF:$<BOOL:${SEIKA_MAIN}>,1,0>
)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../seika)

set(LIBS_TO_LINK "")

# Handle dependencies

if (WIN32)
    list(APPEND LIBS_TO_LINK opengl32)
    if (SEIKA_HTTP_SERVER)
        list(APPEND LIBS_TO_LINK ws2_32)
    endif ()
endif ()

# https://github.com/kuba--/zip
if (NOT TARGET kuba_zip)
    # TODO: Add to thirdparty folder instead
    include(FetchContent)
    FetchContent_Declare(
            kuba_zip_content
            GIT_REPOSITORY https://github.com/kuba--/zip.git
            GIT_TAG v0.3.1
    )
    FetchContent_MakeAvailable(kuba_zip_content)
endif()

if (SEIKA_RENDERING)
    if (NOT TARGET glad)
        add_library(glad STATIC ../thirdparty/glad/glad.c)
        target_include_directories(glad PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty")
    endif()

    if (NOT TARGET stb_image)
        add_library(stb_image STATIC ../thirdparty/stb_image/stb_image.c)
        target_include_directories(stb_image PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty")
    endif()

    if (NOT TARGET freetype)
        # TODO: Add to thirdparty folder instead
        include(FetchContent)
        set(BUILD_SHARED_LIBS OFF)
        FetchContent_Declare(
                freetype_content
                GIT_REPOSITORY https://github.com/freetype/freetype.git
                GIT_TAG VER-2-13-2
        )
        FetchContent_MakeAvailable(freetype_content)
    endif()

    if (NOT TARGET cgltf)
        add_library(cgltf INTERFACE ../thirdparty/cgltf/cgltf.h)
        target_include_directories(cgltf INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/cgltf")
    endif()

    list(APPEND LIBS_TO_LINK glad stb_image freetype cgltf)
endif()

if (SEIKA_AUDIO)
    if (NOT TARGET miniaudio)
        add_library(miniaudio INTERFACE ../thirdparty/miniaudio/miniaudio.h)
        target_include_directories(miniaudio INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/miniaudio")
    endif()

    list(APPEND LIBS_TO_LINK miniaudio)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC zip ${LIBS_TO_LINK})
target_compile_options(${PROJECT_NAME} PUBLIC ${flags})

# Create unit test executable
if (SEIKA_UNIT_TESTS)
    # https://github.com/ThrowTheSwitch/Unity
    if (NOT TARGET unity)
        include(FetchContent)
        add_definitions(-DUNITY_INCLUDE_DOUBLE)
        add_definitions(-DUNITY_SUPPORT_64)

        FetchContent_Declare(
                unity_content
                GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
                GIT_TAG v2.6.0
        )
        FetchContent_MakeAvailable(unity_content)
    endif()

    # Define unit test modules, which should follow '${module}_unit_test.c' for its corresponding file
    set(UNIT_TEST_MODULES
            seika
            array_list_template
            hash_map_template
            event_template
    )

    # Create test executables
    foreach(module ${UNIT_TEST_MODULES})
        set(test_name ${PROJECT_NAME}_${module}_unit_tests)
        set(test_source test/${module}_unit_test.c)

        add_executable(${test_name} ${test_source})
        target_link_libraries(${test_name} ${PROJECT_NAME} unity)
        add_test(NAME ${module}_unit_tests COMMAND ${test_name})
    endforeach()
endif ()
