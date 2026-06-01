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
    int32 nrChannels;
    // format
    GLint internalFormat;
    GLint imageFormat;
    // configuration
    GLint wrapS;
    GLint wrapT;
    bool applyNearestNeighbor;
    char* fileName;
} SkaTexture;

SkaTexture* ska_texture_create(const char* filePath);
SkaTexture* ska_texture_create2(const char* filePath, GLint wrapS, GLint wrapT, bool applyNearestNeighbor);
SkaTexture* ska_texture_create_from_memory(const void* buffer, usize bufferSize);
SkaTexture* ska_texture_create_from_memory2(const void* buffer, usize bufferSize, GLint wrapS, GLint wrapT, bool applyNearestNeighbor);
SkaTexture* ska_texture_create_solid_colored(GLsizei width, GLsizei height, SkaColor color);
SkaTexture* ska_texture_create_checkerboard(GLsizei width, GLsizei height);
void ska_texture_delete(SkaTexture* texture);
GLint ska_texture_wrap_string_to_int(const char* wrap);
const char* ska_texture_get_wrap_s_string(const SkaTexture* texture);
const char* ska_texture_get_wrap_t_string(const SkaTexture* texture);

#ifdef __cplusplus
}
#endif

#endif // #if SKA_RENDERING
