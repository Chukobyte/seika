# Seika

A framework that can be used to make 2D games.

## How to include in a project

Seika uses cmake to build.  To include in your project, add the following to your CMakeLists.txt:
```cmake
# Include seika framework as a dependency
include(FetchContent)

FetchContent_Declare(
        seika
        GIT_REPOSITORY https://github.com/Chukobyte/seika.git
        GIT_TAG main
)

FetchContent_MakeAvailable(seika)
```

Seika uses vcpkg as its package manager, be sure to include this as an option when running cmake:
```
-DCMAKE_TOOLCHAIN_FILE=C:\ProjectWorkspace\C_Plus_Plus\vcpkg\scripts\buildsystems\vcpkg.cmake
```

Make sure to link seika to whatever uses it with `target_link_libraries`.
