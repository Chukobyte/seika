#if SKA_RENDERING

#include "render_context.h"

#include <stddef.h>

#include "seika/memory.h"
#include "seika/assert.h"

static SkaRenderContext *render_context = NULL;

void ska_render_context_initialize(SkaSize2Di resolution) {
  SKA_ASSERT_FMT(render_context == NULL,
                 "Render context is already initialized!");
  render_context = SKA_ALLOC(SkaRenderContext);
  render_context->resolution = resolution;
}

void ska_render_context_finalize() {
  SKA_ASSERT_FMT(render_context != NULL, "Render context is not initialized!");
  SKA_FREE(render_context);
}

SkaRenderContext* ska_render_context_get() {
  SKA_ASSERT_FMT(render_context != NULL, "Render context is not initialized!");
  return render_context;
}

#endif // #if SKA_RENDERING
