#pragma once

#if SKA_RENDERING

#ifdef __cplusplus
extern "C" {
#endif

#include "ska_opengl.h"
#include "seika/math/math.h"

typedef struct SkaTexture {
    GLuint id;
    unsigned char* data;
    GLsizei width;
    GLsizei height;
    int32 nr_channels;
    // format
    GLint internal_format;
    GLint image_format;
    // configuration
    GLint wrap_s;
    GLint wrap_t;
    bool apply_nearest_neighbor;
    char* file_name;
} SkaTexture;

SkaTexture* ska_texture_create(const char* file_path);
SkaTexture* ska_texture_create2(const char* file_path, GLint wrap_s, GLint wrap_t, bool apply_nearest_neighbor);
SkaTexture* ska_texture_create_from_memory(const void* buffer, usize buffer_size);
SkaTexture* ska_texture_create_from_memory2(const void* buffer, usize buffer_size, GLint wrap_s, GLint wrap_t, bool apply_nearest_neighbor);
SkaTexture* ska_texture_create_solid_colored(GLsizei width, GLsizei height, SkaColor color);
SkaTexture* ska_texture_create_checkerboard(GLsizei width, GLsizei height);
// White RGB, alpha = antialiased circular mask (opaque inside the circle, transparent outside).
// Meant to be drawn as a color-tinted sprite (tint via the draw call's own color, not this texture).
SkaTexture* ska_texture_create_circle(GLsizei diameter);
void ska_texture_delete(SkaTexture* texture);
// Also frees the GPU texture, so it needs a current GL context
void ska_texture_delete_with_gl(SkaTexture* texture);
GLint ska_texture_wrap_string_to_int(const char* wrap);
const char* ska_texture_get_wrap_s_string(const SkaTexture* texture);
const char* ska_texture_get_wrap_t_string(const SkaTexture* texture);

#ifdef __cplusplus
}
#endif

#endif // #if SKA_RENDERING
