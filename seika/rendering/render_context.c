#include "render_context.h"

#include <stddef.h>

#include "seika/memory.h"
#include "seika/assert.h"

static SkaRenderContext* renderContext = NULL;

void ska_render_context_initialize() {
    SKA_ASSERT_FMT(renderContext == NULL, "Render context is already initialized!");
    renderContext = SKA_MEM_ALLOCATE(SkaRenderContext);
}

void ska_render_context_finalize() {
    SKA_ASSERT_FMT(renderContext != NULL, "Render context is not initialized!");
    SKA_MEM_FREE(renderContext);
}

SkaRenderContext* ska_render_context_get() {
    SKA_ASSERT_FMT(renderContext != NULL, "Render context is not initialized!");
    return renderContext;
}
