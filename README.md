# Seika

[![windows-msvc-build Actions Status](https://github.com/Chukobyte/seika/workflows/windows-msvc-build/badge.svg)](https://github.com/Chukobyte/seika/actions)
[![windows-mingw-build Actions Status](https://github.com/Chukobyte/seika/workflows/windows-mingw-build/badge.svg)](https://github.com/Chukobyte/seika/actions)
[![ubuntu-gcc-build Actions Status](https://github.com/Chukobyte/seika/workflows/ubuntu-gcc-build/badge.svg)](https://github.com/Chukobyte/seika/actions)
[![macosx-clang-build Actions Status](https://github.com/Chukobyte/seika/workflows/macosx-clang-build/badge.svg)](https://github.com/Chukobyte/seika/actions)

A framework that can be used to make games.

## How to include in a project

Seika uses cmake to build.  To include in your project, add the following to your CMakeLists.txt:
```cmake
# Include seika framework as a dependency
include(FetchContent)

FetchContent_Declare(
        seika
        GIT_REPOSITORY https://github.com/Chukobyte/seika.git
        GIT_TAG v0.0.1
)

FetchContent_MakeAvailable(seika)
```

Seika uses vcpkg as its package manager, be sure to include this as an option when running cmake:
```
-DCMAKE_TOOLCHAIN_FILE=C:\ProjectWorkspace\C_Plus_Plus\vcpkg\scripts\buildsystems\vcpkg.cmake
```

Make sure to link seika to whatever uses it with `target_link_libraries`.

Example projects found [here](https://github.com/Chukobyte/seika-examples).

*Note: Using v0.0.x versions as pre-alpha, won't create full releases until minor version is incremented.*
