#pragma once

#if SKA_RENDERING

#include "seika/defines.h"

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
#include <glad/glad.h>
#elif defined(PLATFORM_ANDROID)
// No loader needed -- GLES3 entry points link directly against libGLESv3.so.
// EGL context/surface setup lives in window/window_android.c, not here.
#include <GLES3/gl3.h>
#elif defined(PLATFORM_EMSCRIPTEN)
#include <GL/gl.h>
#include <GLES3/gl3.h>
#else
#error "Platform not supported"
#endif

// GLES3 core doesn't define GL_CLAMP_TO_BORDER (only desktop GL / GLES 3.2+ core do) -- it's
// available via the near-universally-supported GL_EXT_texture_border_clamp/GL_OES_texture_
// border_clamp extensions, which define the same numeric value for interop. #ifndef makes this a
// no-op on desktop, where glad already provides the real constant.
#ifndef GL_CLAMP_TO_BORDER
#define GL_CLAMP_TO_BORDER 0x812D
#endif

void ska_opengl_print_errors(const char* context);

#endif // #if SKA_RENDERING
