#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include "seika/defines.h"

typedef struct SkaRenderContext {
    FT_Library freeTypeLibrary;
    int32 windowWidth;
    int32 windowHeight;
} SkaRenderContext;

void ska_render_context_initialize();
void ska_render_context_finalize();
SkaRenderContext* ska_render_context_get();
