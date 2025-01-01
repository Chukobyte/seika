#pragma once

#if SKA_UI

#include "seika/defines.h"

void sui_init();
void sui_finalize();
void sui_render();

bool sui_button(const char* label);


#endif // #if SKA_UI
