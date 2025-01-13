#pragma once

#if SKA_UI

#include "seika/math/math.h"

typedef enum SUIWindowFlag {
    SUIWindowFlag_NONE = 0,
    SUIWindowFlag_BORDERLESS = 1 << 0,
} SUIWindowFlag;

typedef struct SUIWindowSettings {
    SkaVector2* position;
    SkaVector2* size;
} SUIWindowSettings;

void sui_init();
void sui_finalize();
void sui_new_frame();
void sui_render();

void sui_register_mouse_position(SkaVector2 position);
void sui_register_window_resize(SkaSize2Di size);

bool sui_begin_window(const char* label, SUIWindowSettings* settings);
void sui_end_window();

bool sui_button(const char* label);


#endif // #if SKA_UI
