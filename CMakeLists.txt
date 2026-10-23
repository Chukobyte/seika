cmake_minimum_required(VERSION 3.27.0)

# FreeType 2.13.2 (fetched below) ships a cmake_minimum_required for a CMake
# version too old for CMake >= 4.0 to accept at all. This tells CMake to treat
# such ancient requirements as this version instead of hard-erroring.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

set(CMAKE_C_STANDARD 11)

project(seika C)

# window_cocoa.m/input_cocoa.m need the Objective-C frontend; gated so non-Apple configures
# never probe for an Objective-C compiler.
if (APPLE)
    enable_language(OBJC)
endif ()

# Optional Modules
option(SEIKA_ECS "Will enable ecs" ON)
option(SEIKA_INPUT "Will enable input" ON)
option(SEIKA_RENDERING "Will enable rendering" ON)
option(SEIKA_AUDIO "Will enable audio" ON)
option(SEIKA_THREAD "Will enable thread library" ON)
option(SEIKA_NETWORK "Will enable networking" OFF)
option(SEIKA_HTTP_SERVER "Enable built-in static HTTP server" ON)
option(SEIKA_MAIN "Enable SKA_MAIN macro to simplify the main function" ON)
option(SEIKA_EXAMPLES "Will enable seika visual example programs" ON)

if (NOT DEFINED SEIKA_UNIT_TESTS)
    option(SEIKA_UNIT_TESTS "Will enable seika unit tests" ON)
endif ()

if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    list(APPEND flags "/W3" "/Zc:preprocessor")
    # MSVC's "unsafe" CRT deprecation warnings (fopen/strcpy/sprintf/etc, C4996) push
    # Microsoft-only _s replacements that don't exist on Linux/macOS -- not viable for
    # this cross-platform codebase. GCC/Clang builds already disable the equivalent
    # via -Wno-deprecated-declarations below; this is the MSVC counterpart.
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
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

file(GLOB_RECURSE SEIKA_SRC CONFIGURE_DEPENDS ./seika/*.c ./seika/*.h)
if (APPLE)
    file(GLOB_RECURSE SEIKA_OBJC_SRC CONFIGURE_DEPENDS ./seika/*.m)
    list(APPEND SEIKA_SRC ${SEIKA_OBJC_SRC})
endif ()
add_library(${PROJECT_NAME} STATIC ${SEIKA_SRC})
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

# GCC 13+ on Linux (Ubuntu 24.04+) hides POSIX symbols (clock_gettime,
# CLOCK_MONOTONIC, nanosleep) under strict -std=c11 unless the feature-test
# macro is set.  Not needed on Windows (uses Win32 API) or Emscripten.
if (NOT WIN32 AND NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    target_compile_definitions(${PROJECT_NAME} PRIVATE _DEFAULT_SOURCE)
endif ()
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../seika)

set(LIBS_TO_LINK "")

# Handle dependencies

if (WIN32)
    list(APPEND LIBS_TO_LINK opengl32 dinput8 dxguid)
    if (SEIKA_HTTP_SERVER)
        list(APPEND LIBS_TO_LINK ws2_32)
    endif ()
elseif (ANDROID)
    # Must be checked before the UNIX AND NOT APPLE branch below: CMake's Android NDK toolchain
    # sets ANDROID=TRUE but UNIX can also be true, and there is no X11 on Android -- letting this
    # fall into the UNIX branch would hard-fail configure on find_package(X11 REQUIRED).
    list(APPEND LIBS_TO_LINK GLESv3 EGL android log)
    # window_android.c (part of THIS target, not just the app-level host) includes
    # <android_native_app_glue.h> -- the include path must be added here, not only on the app's
    # own executable target, or seika itself fails to compile.
    target_include_directories(${PROJECT_NAME} PRIVATE ${ANDROID_NDK}/sources/android/native_app_glue)
elseif (EMSCRIPTEN)
    # Same ordering risk as ANDROID above: the Emscripten toolchain file sets EMSCRIPTEN=TRUE but
    # whether it also sets CMake's own UNIX is inconsistent/version-dependent, so this must also be
    # checked before UNIX AND NOT APPLE. No libs to append -- GL/GLES symbols resolve at link time
    # via emcc's own WebGL glue (see web/CMakeLists.txt's -sUSE_WEBGL2 link flag).
elseif (APPLE)
    list(APPEND LIBS_TO_LINK "-framework OpenGL" "-framework Cocoa" "-framework CoreFoundation")
elseif (UNIX AND NOT APPLE)
    # find_package (vs. hardcoding -lX11/-lGL) gives a clear configure-time error if X11/GL dev
    # headers are missing, instead of a late, more confusing link-time error after a full
    # library compile.
    find_package(X11 REQUIRED)
    find_package(OpenGL REQUIRED)
    # Linux splits libm out from libc (unlike Windows/macOS, where math functions are bundled
    # into the main runtime) -- needed for atan2f/sincosf/tanf/pow etc. used by math.c and
    # thirdparty/stb_image.
    list(APPEND LIBS_TO_LINK ${X11_LIBRARIES} ${OPENGL_LIBRARIES} m)
    target_include_directories(${PROJECT_NAME} PUBLIC ${X11_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR})
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

if (NOT TARGET cjson)
    add_library(cjson STATIC thirdparty/cjson/cJSON.c)
    target_include_directories(cjson PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")
    target_compile_definitions(cjson PUBLIC CJSON_HIDE_SYMBOLS)
endif ()

if (SEIKA_RENDERING)
    # glad is a desktop GL loader (used only by ska_opengl.h's PLATFORM_WINDOWS/LINUX/MACOS
    # branch) and does not compile against the Android NDK sysroot or under emcc -- Android links
    # GLESv3 directly by name (see the ANDROID branch above), and Emscripten resolves GL/GLES
    # symbols itself via its own WebGL glue, no loader needed for either.
    if (NOT ANDROID AND NOT EMSCRIPTEN AND NOT TARGET glad)
        add_library(glad STATIC thirdparty/glad/glad.c)
        target_include_directories(glad PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")
    endif()

    if (NOT TARGET stb_image)
        add_library(stb_image STATIC thirdparty/stb_image/stb_image.c thirdparty/stb_image/stb_image_write_impl.c)
        target_include_directories(stb_image PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")
    endif()

    if (NOT TARGET msf_gif)
        add_library(msf_gif STATIC thirdparty/msf_gif/msf_gif_impl.c)
        target_include_directories(msf_gif PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/msf_gif")
    endif()

    if (NOT TARGET freetype)
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
        add_library(cgltf INTERFACE thirdparty/cgltf/cgltf.h)
        target_include_directories(cgltf INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/cgltf")
    endif()

    list(APPEND LIBS_TO_LINK stb_image msf_gif freetype cgltf)
    if (NOT ANDROID AND NOT EMSCRIPTEN)
        list(APPEND LIBS_TO_LINK glad)
    endif()
endif()

if (SEIKA_AUDIO)
    if (NOT TARGET miniaudio)
        add_library(miniaudio INTERFACE thirdparty/miniaudio/miniaudio.h)
        target_include_directories(miniaudio INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/miniaudio")
    endif()

    list(APPEND LIBS_TO_LINK miniaudio)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC zip ${LIBS_TO_LINK})
target_compile_options(${PROJECT_NAME} PUBLIC ${flags})

# Create unit test executable
if (SEIKA_UNIT_TESTS)
    enable_testing()

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
            hash_map_string
            event_template
            math
            gamepad_db
            ecs_component
            simd
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

# Visual example programs (manual/visual checks, not run by ctest) - each example
# owns its own CMakeLists.txt under examples/<name>/
if (SEIKA_EXAMPLES)
    add_subdirectory(examples)
endif ()

# gamecontrollerdb.txt -> seika gamepad DB table generator (run manually, not via ctest;
# output is committed to seika/seika/input/gamepad_db_generated.c)
option(SEIKA_GAMEPAD_DB_GEN "Build the gamecontrollerdb.txt -> seika gamepad DB generator" OFF)
if (SEIKA_GAMEPAD_DB_GEN)
    add_executable(seika_gamepad_db_gen tools/gen_gamepad_db.c)
    target_link_libraries(seika_gamepad_db_gen PRIVATE ${PROJECT_NAME})
    target_compile_options(seika_gamepad_db_gen PRIVATE ${flags})
endif ()
