#pragma once

#if SKA_RENDERING

#include <ft2build.h>
#include FT_FREETYPE_H

#include "seika/math/math.h"

typedef struct SkaRenderContext {
    SkaSize2Di resolution;
    FT_Library free_type_library;
} SkaRenderContext;

void ska_render_context_initialize(SkaSize2Di resolution);
void ska_render_context_finalize();
SkaRenderContext* ska_render_context_get();

#endif // #if SKA_RENDERING
