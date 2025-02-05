#if SKA_SDL

#include "seika/time.h"

#include <SDL3/SDL.h>

uint32 ska_get_ticks() {
    return (uint32)SDL_GetTicks();
}

void ska_delay(uint32 milliseconds) {
    SDL_Delay(milliseconds);
}

#endif // #if SKA_SDL
