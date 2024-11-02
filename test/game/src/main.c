#include <stdlib.h>

#include <seika/time.h>
#include <seika/ska_sdl.h>
#include <seika/math/math.h>
#include <seika/rendering/window.h>
#include <seika/input/input.h>
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
        const bool shouldQuit = ska_sdl_update();
        if (shouldQuit || ska_input_is_key_just_pressed(SkaInputKey_KEYBOARD_ESCAPE, 0)) {
            break;
        }
        ska_window_render(&(SkaColor){ 0.2f, 0.2f, 0.2f, 1.0f });
        ska_delay(10);
    }
}

int main(int argv, char** args) {
    game_initialize();
    game_run();
    game_finalize();
    return EXIT_SUCCESS;
}
