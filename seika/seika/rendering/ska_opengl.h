#pragma once

#if SKA_RENDERING

#include "seika/defines.h"

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
#include <glad/glad.h>
#elif defined(PLATFORM_EMSCRIPTEN)
#include <GL/gl.h>
#include <GLES3/gl3.h>
#else
#error "Platform not supported"
#endif

void ska_opengl_print_errors(const char* context);

#endif // #if SKA_RENDERING
