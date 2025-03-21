#if SKA_RENDERING

#include "texture.h"

#include <string.h>
#include <memory.h>

#include <stb_image/stb_image.h>

#include "seika/asset/asset_file_loader.h"
#include "seika/memory.h"
#include "seika/string.h"
#include "seika/assert.h"

static const struct SkaTexture DEFAULT_TEXTURE_REF = {
    .id = 0,
    .data = NULL,
    .width = 0,
    .height = 0,
    .nrChannels = 0,
    .internalFormat = GL_RGBA,
    .imageFormat = GL_RGBA,
    .wrapS = GL_CLAMP_TO_BORDER,
    .wrapT = GL_CLAMP_TO_BORDER,
    .applyNearestNeighbor = true,
    .fileName = NULL
};

static inline bool ska_texture_is_texture_valid(const SkaTexture* texture) {
    return texture != NULL;
}

static inline SkaTexture* ska_texture_create_default_texture() {
    SkaTexture* texture = SKA_ALLOC(SkaTexture);
    memcpy(texture, &DEFAULT_TEXTURE_REF, sizeof(SkaTexture));
    return texture;
}

static void ska_texture_generate(SkaTexture* texture);

SkaTexture* ska_texture_create_texture(const char* filePath) {
    return ska_texture_create_texture2(filePath, DEFAULT_TEXTURE_REF.wrapS, DEFAULT_TEXTURE_REF.wrapT, DEFAULT_TEXTURE_REF.applyNearestNeighbor);
}

SkaTexture* ska_texture_create_texture2(const char* filePath, GLint wrapS, GLint wrapT, bool applyNearestNeighbor) {
    SkaTexture* texture = ska_texture_create_default_texture();
    texture->fileName = ska_strdup(filePath);
    texture->wrapS = wrapS;
    texture->wrapT = wrapT;
    texture->applyNearestNeighbor = applyNearestNeighbor;
    // Load image data
    SkaAssetFileImageData* fileImageData = ska_asset_file_loader_load_image_data(filePath);
    SKA_ASSERT_FMT(fileImageData != NULL, "Failed to load texture image at file path '%s'", filePath);
    const usize imageDataSize = strlen((char*) fileImageData->data);
    texture->data = (unsigned char*) SKA_ALLOC_BYTES(imageDataSize);
//    memcpy(texture->data, fileImageData->data, imageDataSize);
    texture->data = fileImageData->data; // TODO: Fix
    texture->width = fileImageData->width;
    texture->height = fileImageData->height;
    texture->nrChannels = fileImageData->nrChannels;

    ska_texture_generate(texture);

    ska_asset_file_loader_free_image_data(fileImageData);

    return texture;
}

SkaTexture* ska_texture_create_texture_from_memory(const void* buffer, usize bufferSize) {
    return ska_texture_create_texture_from_memory2(buffer, bufferSize, DEFAULT_TEXTURE_REF.wrapS,
                                                   DEFAULT_TEXTURE_REF.wrapT, DEFAULT_TEXTURE_REF.applyNearestNeighbor);
}

SkaTexture* ska_texture_create_texture_from_memory2(const void* buffer, usize bufferSize, GLint wrapS, GLint wrapT, bool applyNearestNeighbor) {
    SkaTexture* texture = ska_texture_create_default_texture();
    texture->wrapS = wrapS;
    texture->wrapT = wrapT;
    texture->applyNearestNeighbor = applyNearestNeighbor;
    texture->data = (unsigned char*) SKA_ALLOC_BYTES(bufferSize);
    unsigned char* imageData = stbi_load_from_memory((unsigned char*)buffer, (int32)bufferSize, &texture->width, &texture->height, &texture->nrChannels, 0);
    SKA_ASSERT(imageData);
//    memcpy(texture->data, imageData, bufferSize);
    texture->data = imageData;

    ska_texture_generate(texture);

    stbi_image_free(imageData);

    return texture;
}

SkaTexture* ska_texture_create_solid_colored_texture(GLsizei width, GLsizei height, GLuint colorValue) {
    SkaTexture* texture = ska_texture_create_default_texture();
    texture->nrChannels = 4;
    texture->width = width;
    texture->height = height;

    const GLsizei dataSize = width * height * 4;
    texture->data = (unsigned char*)SKA_ALLOC_BYTES(dataSize);
    for (GLsizei i = 0; i < dataSize; i++) {
        texture->data[i] = colorValue;
    }

    ska_texture_generate(texture);

    return texture;
}

void ska_texture_generate(SkaTexture* texture) {
    // OpenGL Stuff
    if (texture->nrChannels == 1) {
        texture->imageFormat = GL_RED;
    } else if (texture->nrChannels == 3) {
        texture->imageFormat = GL_RGB;
    } else if (texture->nrChannels == 4) {
        texture->imageFormat = GL_RGBA;
    }

    // Create texture
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexImage2D(GL_TEXTURE_2D, 0, texture->internalFormat, texture->width, texture->height, 0, texture->imageFormat, GL_UNSIGNED_BYTE, texture->data);
    glGenerateMipmap(GL_TEXTURE_2D);
    // Wrap and filter modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture->wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture->wrapT);
    // Defaults to bilinear interpolation
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ska_texture_delete(SkaTexture* texture) {
    SKA_FREE(texture->fileName);
    SKA_FREE(texture);
}

GLint ska_texture_wrap_string_to_int(const char* wrap) {
    if (strcmp(wrap, "clamp_to_border") == 0) {
        return GL_CLAMP_TO_BORDER;
    } else if (strcmp(wrap, "repeat") == 0) {
        return GL_REPEAT;
    }
    return GL_CLAMP_TO_BORDER;
}

const char* ska_texture_get_wrap_s_string(const SkaTexture* texture) {
    return texture->wrapS == GL_REPEAT ? "repeat" : "clamp_to_border";
}

const char* ska_texture_get_wrap_t_string(const SkaTexture* texture) {
    return texture->wrapT == GL_REPEAT ? "repeat" : "clamp_to_border";
}

#endif // #if SKA_RENDERING
