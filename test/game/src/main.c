#include <stdlib.h>

#include <seika/asset/asset_manager.h>
#include <seika/audio/audio_manager.h>
#include <seika/time.h>
#include <seika/ska_sdl.h>
#include <seika/math/math.h>
#include <seika/rendering/window.h>
#include <seika/input/input.h>
#include <seika/rendering/renderer.h>
#include <seika/audio/audio.h>
#include <seika/assert.h>

static SkaAudioSource* audioSource = NULL;

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
    ska_audio_initialize();
    ska_asset_manager_initialize();
    audioSource = ska_asset_manager_load_audio_source_wav("test/game/assets/audio/rainbow_orb.wav", "orb");
    SKA_ASSERT(audioSource);
}

static void game_finalize() {
    ska_window_finalize();
    ska_input_finalize();
    ska_audio_finalize();
    ska_asset_manager_finalize();
}

static void game_run() {
    while (true) {
        ska_input_new_frame();
        const bool shouldQuit = ska_sdl_update();
        if (shouldQuit || ska_input_is_key_just_pressed(SkaInputKey_KEYBOARD_ESCAPE, 0)) {
            break;
        }
        if (ska_input_is_key_just_pressed(SkaInputKey_KEYBOARD_SPACE, 0)) {
            ska_audio_manager_play_sound2("orb", false);
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
