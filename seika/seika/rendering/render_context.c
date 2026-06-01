#if SKA_RENDERING

#include "render_context.h"

#include <stddef.h>

#include "seika/memory.h"
#include "seika/assert.h"

static SkaRenderContext* renderContext = NULL;

void ska_render_context_initialize(SkaSize2Di resolution) {
    SKA_ASSERT_FMT(renderContext == NULL, "Render context is already initialized!");
    renderContext = SKA_ALLOC(SkaRenderContext);
    renderContext->resolution = resolution;
}

void ska_render_context_finalize() {
    SKA_ASSERT_FMT(renderContext != NULL, "Render context is not initialized!");
    SKA_FREE(renderContext);
}

SkaRenderContext* ska_render_context_get() {
    SKA_ASSERT_FMT(renderContext != NULL, "Render context is not initialized!");
    return renderContext;
}

#endif // #if SKA_RENDERING
