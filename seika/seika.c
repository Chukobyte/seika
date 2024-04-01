#include "seika.h"

#include <time.h>

#include <SDL3/SDL.h>
#include <glad/glad.h>

#include "flag_utils.h"
#include "logger.h"
#include "input/sdl_input.h"
#include "seika/assert.h"
#include "seika/rendering/renderer.h"
#include "seika/audio/audio_manager.h"
#include "seika/asset/asset_manager.h"

#define SKA_AUDIO_SOURCE_DEFAULT_WAV_SAMPLE_RATE 44100
#define SKA_WINDOW_DEFAULT_MAINTAIN_ASPECT_RATIO false

typedef enum SkaSystemFlag {
    SkaSystemFlag_NONE = 0,
    SkaSystemFlag_CORE = 1 << 0,
    SkaSystemFlag_WINDOW = 2 << 0,
    SkaSystemFlag_INPUT = 3 << 0,
    SkaSystemFlag_AUDIO = 4 << 0,
    SkaSystemFlag_ALL = SkaSystemFlag_CORE | SkaSystemFlag_WINDOW | SkaSystemFlag_INPUT | SkaSystemFlag_AUDIO,
} SkaSystemFlag;

typedef struct SkaState {
    SkaSystemFlag runningSystems;
    bool shutdownRequested;
} SkaState;

static SkaState skaState = { .runningSystems = SkaSystemFlag_NONE, .shutdownRequested = false };

static SDL_Window* window = NULL;
static SDL_GLContext openGlContext = NULL;

bool ska_init() {
    if (SKA_HAS_FLAG(SkaSystemFlag_CORE, skaState.runningSystems)) {
        return false;
    }

    // Set random seed
    srand((int)time(NULL));

    if (SDL_Init(0) != 0) {
        return false;
    }

    SKA_ADD_FLAGS(skaState.runningSystems, SkaSystemFlag_CORE);
    return true;
}

void ska_shutdown() {
    if (SKA_HAS_FLAG(SkaSystemFlag_CORE, skaState.runningSystems)) {
        SDL_Quit();
        SKA_REMOVE_FLAGS(skaState.runningSystems, SkaSystemFlag_CORE);
    }
}

bool ska_init_all(const char* title, int32 windowWidth, int32 windowHeight, int32 resolutionWidth, int32 resolutionHeight) {
    return ska_init_all2(title, windowWidth, windowHeight, resolutionWidth, resolutionHeight, SKA_AUDIO_SOURCE_DEFAULT_WAV_SAMPLE_RATE, SKA_WINDOW_DEFAULT_MAINTAIN_ASPECT_RATIO);
}

bool ska_init_all2(const char* title, int32 windowWidth, int32 windowHeight, int32 resolutionWidth, int32 resolutionHeight, uint32 audioWavSampleRate, bool maintainAspectRatio) {
    if (!ska_init()) {
        return false;
    }

    if (!ska_window_init2(title, windowWidth, windowHeight, resolutionWidth, resolutionHeight, maintainAspectRatio)) {
        return false;
    }

    if (!ska_input_init()) {
        return false;
    }

    if (!ska_audio_init2(audioWavSampleRate)) {
        return false;
    }

    ska_asset_manager_initialize();

    return true;
}


void ska_shutdown_all() {
    ska_window_shutdown();
    ska_input_shutdown();
    ska_audio_shutdown();
    ska_asset_manager_finalize();
    ska_shutdown();
}

void ska_update() {
    SKA_ASSERT(SKA_HAS_FLAG(SkaSystemFlag_CORE, skaState.runningSystems));
    ska_input_new_frame();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_EVENT_QUIT:
                skaState.shutdownRequested = true;
                break;
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
}

void ska_fixed_update(f32 deltaTime) {
    SKA_ASSERT(SKA_HAS_FLAG(SkaSystemFlag_CORE, skaState.runningSystems));

    static f32 globalTime = 0.0f;
    globalTime += deltaTime;
    ska_renderer_set_global_shader_param_time(globalTime);
}

bool ska_is_running() {
    return !skaState.shutdownRequested;
}

uint32 ska_get_ticks() {
    SKA_ASSERT(SKA_HAS_FLAG(SkaSystemFlag_CORE, skaState.runningSystems));
    return (uint32)SDL_GetTicks();
}

void ska_delay(uint32 timeToWait) {
    SKA_ASSERT(SKA_HAS_FLAG(SkaSystemFlag_CORE, skaState.runningSystems));
    SDL_Delay(timeToWait);
}

bool ska_set_vsync_enabled(bool enabled) {
    SKA_ASSERT(SKA_HAS_FLAG(SkaSystemFlag_CORE, skaState.runningSystems));
    return SDL_GL_SetSwapInterval((int)enabled) == 0;
}

bool ska_print_errors() {
    return ska_logger_internal_print_queue();
}

bool ska_window_init(const char* title, int32 windowWidth, int32 windowHeight) {
    return ska_window_init2(title, windowWidth, windowHeight, windowWidth, windowHeight, SKA_WINDOW_DEFAULT_MAINTAIN_ASPECT_RATIO);
}

bool ska_window_init2(const char* title, int32 windowWidth, int32 windowHeight, int32 resolutionWidth, int32 resolutionHeight, bool maintainAspectRatio) {
    SKA_ASSERT(SKA_HAS_FLAG(SkaSystemFlag_CORE, skaState.runningSystems));
    if (SKA_HAS_FLAG(SkaSystemFlag_WINDOW, skaState.runningSystems)) {
        return false;
    }

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
        return false;
    }

    // OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Create window
    const uint32 windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    window = SDL_CreateWindow(
            title,
            windowWidth,
            windowHeight,
            windowFlags
    );
    if (!window) {
        return false;
    }

    // Create OpenGL Context
    openGlContext = SDL_GL_CreateContext(window);

    // Initialize Glad
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        return false;
    }

    // Initialize rendering
    ska_renderer_initialize(windowWidth, windowHeight, resolutionWidth, resolutionHeight, maintainAspectRatio);

    SKA_ADD_FLAGS(skaState.runningSystems, SkaSystemFlag_WINDOW);
    return true;
}

void ska_window_shutdown() {
    if (SKA_HAS_FLAG(SkaSystemFlag_WINDOW, skaState.runningSystems)) {
        SDL_GL_DeleteContext(openGlContext);
        SDL_DestroyWindow(window);
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        SKA_REMOVE_FLAGS(skaState.runningSystems, SkaSystemFlag_WINDOW);
    }
}

void ska_window_render() {
    SKA_ASSERT(SKA_HAS_FLAG(SkaSystemFlag_WINDOW, skaState.runningSystems));

    static const SkaColor backgroundColor = {33.0f / 255.0f, 33.0f / 255.0f, 33.0f / 255.0f, 1.0f };
    ska_renderer_process_and_flush_batches(&backgroundColor);

    SDL_GL_SwapWindow(window);
}

bool ska_input_init() {
    SKA_ASSERT(SKA_HAS_FLAG(SkaSystemFlag_CORE, skaState.runningSystems));
    if (SDL_InitSubSystem( SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD) != 0) {
        return false;
    }

    if (!ska_sdl_load_gamepad_mappings()) {
        return false;
    }

    return true;
}

void ska_input_shutdown() {
    if (SKA_HAS_FLAG(SkaSystemFlag_INPUT, skaState.runningSystems)) {
        SKA_REMOVE_FLAGS(skaState.runningSystems, SkaSystemFlag_INPUT);
        SDL_QuitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD);
    }
}

bool ska_audio_init() {
    return ska_audio_init2(SKA_AUDIO_SOURCE_DEFAULT_WAV_SAMPLE_RATE);
}

bool ska_audio_init2(uint32 audioWavSampleRate) {
    SKA_ASSERT(SKA_HAS_FLAG(SkaSystemFlag_CORE, skaState.runningSystems));
    return ska_audio_manager_init(audioWavSampleRate);
}

void ska_audio_shutdown() {
    if (SKA_HAS_FLAG(SkaSystemFlag_AUDIO, skaState.runningSystems)) {
        SKA_REMOVE_FLAGS(skaState.runningSystems, SkaSystemFlag_AUDIO);
    }
}
