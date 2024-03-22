#include "seika.h"

#include <SDL3/SDL.h>

#include "flag_utils.h"
#include "new-seika/assert.h"

#define SKA_AUDIO_SOURCE_DEFAULT_WAV_SAMPLE_RATE 44100
#define SKA_WINDOW_DEFAULT_MAINTAIN_ASPECT_RATIO false

typedef enum SkaSystemFlag {
    SkaSystemFlag_NONE = 0,
    SkaSystemFlag_WINDOW = 1 << 0,
    SkaSystemFlag_INPUT = 2 << 0,
    SkaSystemFlag_AUDIO = 2 << 0,
    SkaSystemFlag_ALL = SkaSystemFlag_WINDOW | SkaSystemFlag_INPUT | SkaSystemFlag_AUDIO,
} SkaSystemFlag;

typedef struct SkaState {
    SkaSystemFlag runningSystems;
} SkaState;

static SkaState skaState = { .runningSystems = SkaSystemFlag_NONE };

bool ska_init_all(const char* title, int32 windowWidth, int32 windowHeight, int32 resolutionWidth, int32 resolutionHeight) {
    return ska_init_all2(title, windowWidth, windowHeight, resolutionWidth, resolutionHeight, SKA_AUDIO_SOURCE_DEFAULT_WAV_SAMPLE_RATE, SKA_WINDOW_DEFAULT_MAINTAIN_ASPECT_RATIO);
}

bool ska_init_all2(const char* title, int32 windowWidth, int32 windowHeight, int32 resolutionWidth, int32 resolutionHeight, uint32 audioWavSampleRate, bool maintainAspectRatio) {
    if (!ska_window_init2(title, windowWidth, windowHeight, resolutionWidth, resolutionHeight, maintainAspectRatio)) {
        return false;
    }

    if (!ska_input_init()) {
        return false;
    }

    if (!ska_audio_init2(audioWavSampleRate)) {
        return false;
    }

    return true;
}


void ska_shutdown_all() {
    ska_window_shutdown();
    ska_input_shutdown();
    ska_audio_shutdown();
}

bool ska_is_running() {
    return skaState.runningSystems != SkaSystemFlag_NONE;
}

uint64 ska_get_ticks() {
    return SDL_GetTicks();
}

void ska_delay(uint32 timeToWait) {
    SDL_Delay(timeToWait);
}

bool ska_set_vsync_enabled(bool enabled) {
    return SDL_GL_SetSwapInterval((int)enabled) == 0;
}


bool ska_window_init(const char* title, int32 windowWidth, int32 windowHeight) {
    return ska_window_init2(title, windowWidth, windowHeight, windowWidth, windowHeight, SKA_WINDOW_DEFAULT_MAINTAIN_ASPECT_RATIO);
}

bool ska_window_init2(const char* title, int32 windowWidth, int32 windowHeight, int32 resolutionWidth, int32 resolutionHeight, bool maintainAspectRatio) {
    if (SKA_HAS_FLAG(SkaSystemFlag_WINDOW, skaState.runningSystems)) {
        return false;
    }

    SKA_ADD_FLAGS(skaState.runningSystems, SkaSystemFlag_WINDOW);
    return true;
}

void ska_window_shutdown() {
    if (SKA_HAS_FLAG(SkaSystemFlag_WINDOW, skaState.runningSystems)) {
        SKA_REMOVE_FLAGS(skaState.runningSystems, SkaSystemFlag_WINDOW);
    }
}

void ska_window_render() {
    SKA_ASSERT(SKA_HAS_FLAG(SkaSystemFlag_WINDOW, skaState.runningSystems));
}

bool ska_input_init() {
    return true;
}

void ska_input_shutdown() {
    if (SKA_HAS_FLAG(SkaSystemFlag_INPUT, skaState.runningSystems)) {
        SKA_REMOVE_FLAGS(skaState.runningSystems, SkaSystemFlag_INPUT);
    }
}

void ska_input_update() {
    SKA_ASSERT(SKA_HAS_FLAG(SkaSystemFlag_INPUT, skaState.runningSystems));
}

bool ska_audio_init() {
    return true;
}

void ska_audio_shutdown() {
    if (SKA_HAS_FLAG(SkaSystemFlag_AUDIO, skaState.runningSystems)) {
        SKA_REMOVE_FLAGS(skaState.runningSystems, SkaSystemFlag_AUDIO);
    }
}
