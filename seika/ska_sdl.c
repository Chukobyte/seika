#include "ska_sdl.h"

#include <SDL3/SDL.h>

#if SKA_INPUT
#include "input/sdl_input.h"
#endif

#if SKA_RENDERING
#include "rendering/renderer.h"
#endif

bool ska_sdl_update() {
    bool shouldQuit = false;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_EVENT_QUIT: {
                shouldQuit = true;
                break;
            }
            case SDL_EVENT_WINDOW_RESIZED: {
#if SKA_RENDERING
                const Sint32 windowWidth = event.window.data1;
                const Sint32 windowHeight = event.window.data2;
                ska_renderer_update_window_size(windowWidth, windowHeight);
#endif
                break;
            }
            default: {
#if SKA_INPUT
                ska_sdl_process_event(event);
#endif
                break;
            }
        }
    }
#if SKA_INPUT
    ska_sdl_process_axis_events();
#endif
    return shouldQuit;
}
