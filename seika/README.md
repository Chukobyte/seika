# Seika

A framework for windows, macOS, linux, web (emscripten), android, and iphone that can be used to make games.

## Tech Stack

- [C11](https://en.wikipedia.org/wiki/C_(programming_language))
- [OpenGL 3.3](https://www.opengl.org/)
- [miniaudio](https://github.com/mackron/miniaudio) - Audio
- [freetype](https://github.com/freetype/freetype) - Font
- [unity](https://github.com/ThrowTheSwitch/Unity) - Unit Tests
- [zip](https://github.com/kuba--/zip) - Reading archives
- [stb image](https://github.com/nothings/stb) - Image loading

## Current Features

- [x] 2D Sprite Renderer
- [x] Font Renderer
- [x] 3D Renderer
- [x] Customizable shaders (with custom langauge)
- [x] Audio
- [x] Networking (udp only for now)
- [x] pthread wrapper and thread pool
- [x] ECS
- [x] Multiple data structures such as hashmap and array list (vector)

## How to include in a project

Seika uses cmake to build.  To include in your project, add the following to your CMakeLists.txt:

## Example

A simple example of creating a window and querying for inputs.

```c
#include <stdio.h>

#include <seika/seika.h>
#include <seika/input/input.h>

int main(int argv, char** args) {
    ska_window_initialize((SkaWindowProperties){
        .title = "Simple Window",
        .windowWidth = 800,
        .windowHeight = 600,
        .resolutionWidth = 800,
        .resolutionHeight = 600,
        .maintainAspectRatio = true,
    });
    ska_input_initialize();

    while (true) {
        // TODO: Need to update this
        ska_update();

        if (ska_input_is_key_just_pressed(SkaInputKey_KEYBOARD_ESCAPE, 0)) {
            break;
        }
        if (ska_input_is_key_just_pressed(SkaInputKey_KEYBOARD_SPACE, 0)) {
            printf("space just pressed\n");
        }
        if (ska_input_is_key_pressed(SkaInputKey_KEYBOARD_SPACE, 0)) {
            printf("space pressed\n");
        }
        if (ska_input_is_key_just_released(SkaInputKey_KEYBOARD_SPACE, 0)) {
            printf("space just released\n");
        }

        static SkaColor windowBackgroundColor = (SkaColor){ 0.2f, 0.2f, 0.2f, 1.0f };
        ska_window_render(&windowBackgroundColor);
    }

    ska_window_finalize();
    ska_input_finalize();

    return 0;
}
```
