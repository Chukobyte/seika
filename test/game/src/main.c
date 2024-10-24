#include <stdlib.h>

#include <SDL3/SDL.h>

#include <seika/math/math.h>
#include <seika/rendering/window.h>
#include <seika/input/input.h>
#include <seika/input/sdl_input.h>
#include <seika/rendering/renderer.h>

static void game_initialize() {
    ska_window_initialize((SkaWindowProperties){
        .title = "Hello World!",
        .windowWidth = 800,
        .windowHeight = 600,
        .resolutionWidth = 800,
        .resolutionHeight = 600,
        .maintainAspectRatio = true,
    });
    ska_input_initialize();
}

static void game_finalize() {
    ska_window_finalize();
    ska_input_finalize();
}

static void game_run() {
    while (true) {
        ska_input_new_frame();

        bool shouldQuit = false;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_EVENT_QUIT: {
                    shouldQuit = true;
                    break;
                }
                case SDL_EVENT_WINDOW_RESIZED: {
                    const Sint32 windowWidth = event.window.data1;
                    const Sint32 windowHeight = event.window.data2;
                    ska_renderer_update_window_size(windowWidth, windowHeight);
                    break;
                }
                default: {
                    ska_sdl_process_event(event);
                    break;
                }
            }
        }
        ska_sdl_process_axis_events();

        if (shouldQuit || ska_input_is_key_just_pressed(SkaInputKey_KEYBOARD_ESCAPE, 0)) {
            break;
        }
        ska_window_render(&(SkaColor){ 0.2f, 0.2f, 0.2f, 1.0f });
        SDL_Delay(10);
    }
}

int main(int argv, char** args) {
    game_initialize();
    game_run();
    game_finalize();
    return EXIT_SUCCESS;
}
