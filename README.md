# Seika

A framework for windows, macOS, linux, web (emscripten), and android that can be used to make games.

## Tech Stack

- [C11](https://en.wikipedia.org/wiki/C_(programming_language))
- [OpenGL 3.3](https://www.opengl.org/) (GLES3 on android/emscripten)
- [miniaudio](https://github.com/mackron/miniaudio) - Audio
- [freetype](https://github.com/freetype/freetype) - Font
- [unity](https://github.com/ThrowTheSwitch/Unity) - Unit Tests
- [zip](https://github.com/kuba--/zip) - Reading archives
- [stb image](https://github.com/nothings/stb) - Image loading

Windowing and input are implemented natively per platform (Win32/DirectInput, X11, Cocoa, Android, Emscripten) — there is no SDL dependency.

## Current Features

- [x] 2D Sprite Renderer
- [x] Font Renderer
- [x] 3D Renderer (with glTF model loading)
- [x] Customizable shaders (with custom language)
- [x] Audio (with spatial audio)
- [x] Input (keyboard/mouse/gamepad, with a bindable input-action layer and a built-in gamepad database)
- [x] Networking (UDP, plus a built-in static HTTP server)
- [x] pthread wrapper and thread pool
- [x] ECS
- [x] SIMD helpers
- [x] Math (vectors, matrices, quaternions, curves/easing)
- [x] Screen capture (GIF)
- [x] Multiple data structures such as hashmap and array list (vector)

## How to include in a project

Seika uses cmake to build.  To include in your project, add the following to your CMakeLists.txt:

```cmake
add_subdirectory(seika)
target_link_libraries(${PROJECT_NAME} PRIVATE seika)
```

## Example

A simple example of creating a window and querying for inputs.

```c
#include <stdio.h>

#include <seika/seika_main.h>
#include <seika/window/window.h>
#include <seika/rendering/renderer.h>
#include <seika/input/input.h>

SKA_MAIN {
    SkaWindow* window = ska_win_create_window((SkaCreateWindowProps){
        .title = "Simple Window", .pos_x = 100, .pos_y = 100, .width = 800, .height = 600
    });
    ska_renderer_init((SkaRendererInitProps){
        .window_width = 800, .window_height = 600,
        .resolution_width = 800, .resolution_height = 600,
        .maintain_aspect_ratio = true, .init_2d = true,
    });
    ska_input_initialize();

    while (ska_win_is_active(window)) {
        ska_win_process_events(window);

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

        ska_renderer_process_and_flush_batches(&(SkaColor){ .r = 0.2f, .g = 0.2f, .b = 0.2f, .a = 1.0f });
        ska_win_swap_window(window);
        ska_input_new_frame();
    }

    ska_input_finalize();
    ska_renderer_deinit();
    ska_win_destroy_window(window);
    return 0;
}
```
