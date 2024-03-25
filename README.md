# Seika

[![windows-msvc-build Actions Status](https://github.com/Chukobyte/seika/workflows/windows-msvc-build/badge.svg)](https://github.com/Chukobyte/seika/actions)
[![windows-mingw-build Actions Status](https://github.com/Chukobyte/seika/workflows/windows-mingw-build/badge.svg)](https://github.com/Chukobyte/seika/actions)
[![ubuntu-gcc-build Actions Status](https://github.com/Chukobyte/seika/workflows/ubuntu-gcc-build/badge.svg)](https://github.com/Chukobyte/seika/actions)
[![macosx-clang-build Actions Status](https://github.com/Chukobyte/seika/workflows/macosx-clang-build/badge.svg)](https://github.com/Chukobyte/seika/actions)

A framework for windows, macOS, and linux that can be used to make games.

## Tech Stack

- [C11](https://en.wikipedia.org/wiki/C_(programming_language))
- [OpenGL 3.3](https://www.opengl.org/)
- [SDL3](https://github.com/libsdl-org/SDL) - Window, input, and other misc uses
- [clgm](https://github.com/recp/cglm) - Math
- [miniaudio](https://github.com/mackron/miniaudio) - Audio
- [freetype](https://github.com/freetype/freetype) - Font
- [unity](https://github.com/ThrowTheSwitch/Unity) - Unit Tests
- [zip](https://github.com/kuba--/zip) - Reading archives
- [stb image](https://github.com/nothings/stb) - Image loading
- [cJson](https://github.com/DaveGamble/cJSON) - Reading json files

## Current Features

- [x] 2D Sprite Renderer
- [x] Font Renderer
- [x] Customizable shaders (with custom langauge)
- [x] Audio
- [x] Networking (udp only for now)
- [x] pthread wrapper and thread pool
- [x] Math Library
- [x] ECS
- [x] Multiple data structures such as hashmap and array list (vector)

## How to include in a project

Seika uses cmake to build.  To include in your project, add the following to your CMakeLists.txt:
```cmake
# Include seika framework as a dependency
include(FetchContent)

FetchContent_Declare(
        seika
        GIT_REPOSITORY https://github.com/Chukobyte/seika.git
        GIT_TAG v0.1.0
)

FetchContent_MakeAvailable(seika)
```

Make sure to link seika to the target with `target_link_libraries`.

```cmake
target_link_libraries(${PROJECT_NAME} seika)
```

Example projects found [here](https://github.com/Chukobyte/seika-examples).
