#include "ska_sdl.h"

#include <SDL3/SDL.h>

#include "ui/ui.h"

#if SKA_INPUT
#include "input/sdl_input.h"
#endif

#if SKA_RENDERING
#include "rendering/renderer.h"
#endif

#if SKA_UI
void sui_sdl_event(SDL_Event* event) {

}
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
                const Sint32 windowWidth = event.window.data1;
                const Sint32 windowHeight = event.window.data2;
#if SKA_RENDERING
                ska_renderer_update_window_size(windowWidth, windowHeight);
#endif
#if SKA_UI
                sui_register_window_resize((SkaSize2Di){ .w = windowWidth, .h = windowHeight });
#endif
                break;
            }
            default: {
#if SKA_INPUT
                ska_sdl_process_event(event);
#endif
#if SKA_UI
                sui_sdl_event(&event);
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
