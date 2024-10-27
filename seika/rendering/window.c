#if SKA_RENDERING

#include "window.h"

#include <glad/glad.h>
#include <SDL3/SDL.h>

#include "renderer.h"
#include "seika/assert.h"

static SDL_Window* window = NULL;
static SDL_GLContext glContext;
static bool isWindowActive = false;

bool ska_window_initialize(SkaWindowProperties props) {
    SKA_ASSERT(isWindowActive == false);

    const bool isSDLInitialized = SDL_WasInit(0) != 0;
    if (isSDLInitialized) {
        if (SDL_InitSubSystem( SDL_INIT_VIDEO) != 0) {
            return false;
        }
    } else {
        if (SDL_Init( SDL_INIT_VIDEO) != 0) {
            return false;
        }
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
        props.title,
        props.windowWidth,
        props.windowHeight,
        windowFlags
    );
    if (!window) {
        return false;
    }

    // Create OpenGL Context
    glContext = SDL_GL_CreateContext(window);

    // Initialize Glad
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        return false;
    }

    // Initialize rendering
    ska_renderer_initialize(props.windowWidth, props.windowHeight, props.resolutionWidth, props.resolutionHeight, props.maintainAspectRatio);

    isWindowActive = true;
    return true;
}

void ska_window_finalize() {
    SKA_ASSERT(isWindowActive);
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    const bool subsystemsStillInitialized = SDL_WasInit(0) != 0;
    if (!subsystemsStillInitialized) {
        SDL_Quit();
    }

    window = NULL;
}

void ska_window_render(const SkaColor* backgroundColor) {
    SKA_ASSERT(isWindowActive);
    ska_renderer_process_and_flush_batches(backgroundColor);
    SDL_GL_SwapWindow(window);
}

bool ska_window_set_vsync(bool enabled) {
    return SDL_GL_SetSwapInterval((int)enabled) == 0;
}

#endif // #if SKA_RENDERING
