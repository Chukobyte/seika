#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

#include <glad/glad.h>

#include "seika/defines.h"

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

SkaTexture* ska_texture_create_texture(const char* filePath);
SkaTexture* ska_texture_create_texture2(const char* filePath, GLint wrapS, GLint wrapT, bool applyNearestNeighbor);
SkaTexture* ska_texture_create_texture_from_memory(void* buffer, usize bufferSize);
SkaTexture* ska_texture_create_texture_from_memory2(void* buffer, usize bufferSize, GLint wrapS, GLint wrapT, bool applyNearestNeighbor);
SkaTexture* ska_texture_create_solid_colored_texture(GLsizei width, GLsizei height, GLuint colorValue);
void ska_texture_delete(SkaTexture* texture);
GLint ska_texture_wrap_string_to_int(const char* wrap);
const char* ska_texture_get_wrap_s_string(SkaTexture* texture);
const char* ska_texture_get_wrap_t_string(SkaTexture* texture);

#ifdef __cplusplus
}
#endif
