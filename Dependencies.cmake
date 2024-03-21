include(FetchContent)

if (NOT TARGET SDL3::SDL3-static)
    set(SDL_STATIC ON)
    set(SDL_SHARED OFF)

    FetchContent_Declare(
            SDL_content
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
#            GIT_TAG release-2.30.1
            GIT_TAG cacac6cc341d5856d1857bdcf7390551eed54865
    )
    FetchContent_MakeAvailable(SDL_content)
endif ()

if (NOT TARGET glad)
    add_library(glad thirdparty/glad/glad.c)
    target_include_directories(glad PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")
endif()

if (NOT TARGET cglm)
    FetchContent_Declare(
            cglm_content
            GIT_REPOSITORY https://github.com/recp/cglm.git
            GIT_TAG v0.9.2
    )
    FetchContent_MakeAvailable(cglm_content)
endif()

if (NOT TARGET cjson)
    FetchContent_Declare(
            cjson_content
            GIT_REPOSITORY https://github.com/DaveGamble/cJSON.git
            GIT_TAG v1.7.17
    )
    FetchContent_MakeAvailable(cglm_content)
endif()

if (NOT TARGET kuba_zip)
    include(FetchContent)
    FetchContent_Declare(
            kuba_zip_content
            GIT_REPOSITORY https://github.com/kuba--/zip.git
            GIT_TAG v0.3.1
    )
    FetchContent_MakeAvailable(kuba_zip_content)
endif()

if (NOT TARGET miniaudio)
    include(FetchContent)
    FetchContent_Declare(
            miniaudio_content
            GIT_REPOSITORY https://github.com/mackron/miniaudio.git
            GIT_TAG 0.11.21
    )
    FetchContent_MakeAvailable(miniaudio_content)
endif()

if (NOT TARGET stb_image)
    include(FetchContent)
    FetchContent_Declare(
            stb_image_content
            GIT_REPOSITORY https://github.com/nothings/stb.git
            GIT_TAG ae721c50eaf761660b4f90cc590453cdb0c2acd0
    )
    FetchContent_MakeAvailable(stb_image_content)
endif()

if (NOT TARGET unity)
    include(FetchContent)
    FetchContent_Declare(
            unity_content
            GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
            GIT_TAG v2.6.0
    )
    FetchContent_MakeAvailable(unity_content)
endif()

if (NOT TARGET freetype)
    include(FetchContent)
    FetchContent_Declare(
            freetype_content
            GIT_REPOSITORY https://github.com/freetype/freetype.git
            GIT_TAG VER-2-13-2
    )
    FetchContent_MakeAvailable(freetype_content)
endif()
