#if SKA_RENDERING

#include "texture.h"

#include <string.h>
#include <memory.h>
#include <math.h>

#include <stb_image/stb_image.h>

#include "seika/asset/asset_file_loader.h"
#include "seika/memory.h"
#include "seika/string.h"
#include "seika/assert.h"
#include "seika/simd/simd.h"

static const struct SkaTexture default_texture_ref = {
    .id = 0,
    .data = NULL,
    .width = 0,
    .height = 0,
    .nr_channels = 0,
    .internal_format = GL_RGBA,
    .image_format = GL_RGBA,
    .wrap_s = GL_CLAMP_TO_BORDER,
    .wrap_t = GL_CLAMP_TO_BORDER,
    .apply_nearest_neighbor = true,
    .file_name = NULL};

static inline bool ska_texture_is_texture_valid(const SkaTexture* texture) {
    return texture != NULL;
}

static inline SkaTexture* ska_texture_create_default_texture() {
    SkaTexture* texture = SKA_ALLOC(SkaTexture);
    memcpy(texture, &default_texture_ref, sizeof(SkaTexture));
    return texture;
}

static void ska_texture_generate(SkaTexture* texture);

SkaTexture *ska_texture_create(const char *file_path) {
  return ska_texture_create2(file_path, default_texture_ref.wrap_s,
                             default_texture_ref.wrap_t,
                             default_texture_ref.apply_nearest_neighbor);
}

SkaTexture *ska_texture_create2(const char *file_path, GLint wrap_s,
                                GLint wrap_t, bool apply_nearest_neighbor) {
  SkaTexture *texture = ska_texture_create_default_texture();
  texture->file_name = ska_strdup(file_path);
  texture->wrap_s = wrap_s;
  texture->wrap_t = wrap_t;
  texture->apply_nearest_neighbor = apply_nearest_neighbor;
  // Load image data
  SkaAssetFileImageData *file_image_data =
      ska_asset_file_loader_load_image_data(file_path);
  SKA_ASSERT_FMT(file_image_data != NULL,
                 "Failed to load texture image at file path '%s'", file_path);
  SKA_ASSERT_FMT(file_image_data->data != NULL,
                 "Failed to load texture image data at file path '%s'",
                 file_path);
  const usize image_data_size = strlen((char *)file_image_data->data);
  texture->data = (unsigned char *)SKA_ALLOC_BYTES(image_data_size);
  //    memcpy(texture->data, fileImageData->data, imageDataSize);
  texture->data = file_image_data->data; // TODO: Fix
  texture->width = file_image_data->width;
  texture->height = file_image_data->height;
  texture->nr_channels = file_image_data->nr_channels;

  ska_texture_generate(texture);

  ska_asset_file_loader_free_image_data(file_image_data);

  return texture;
}

SkaTexture *ska_texture_create_from_memory(const void *buffer,
                                           usize buffer_size) {
  return ska_texture_create_from_memory2(
      buffer, buffer_size, default_texture_ref.wrap_s, default_texture_ref.wrap_t,
      default_texture_ref.apply_nearest_neighbor);
}

SkaTexture *ska_texture_create_from_memory2(const void *buffer,
                                            usize buffer_size, GLint wrap_s,
                                            GLint wrap_t,
                                            bool apply_nearest_neighbor) {
  SkaTexture *texture = ska_texture_create_default_texture();
  texture->wrap_s = wrap_s;
  texture->wrap_t = wrap_t;
  texture->apply_nearest_neighbor = apply_nearest_neighbor;
  texture->data = (unsigned char *)SKA_ALLOC_BYTES(buffer_size);
  unsigned char *image_data = stbi_load_from_memory(
      (unsigned char *)buffer, (int32)buffer_size, &texture->width,
      &texture->height, &texture->nr_channels, 0);
  SKA_ASSERT(image_data);
  //    memcpy(texture->data, imageData, bufferSize);
  texture->data = image_data;

  ska_texture_generate(texture);

  stbi_image_free(image_data);

  return texture;
}

SkaTexture* ska_texture_create_solid_colored(GLsizei width, GLsizei height, SkaColor color) {
    SkaTexture* texture = ska_texture_create_default_texture();
    texture->nr_channels = 4;
    texture->width = width;
    texture->height = height;
    const GLsizei data_size = width * height * 4;
    texture->data = (unsigned char *)SKA_ALLOC_BYTES(data_size);

    // Every pixel is identical, so the whole buffer is one repeated 4-byte pattern -- fill it
    // in one SIMD-wide pass instead of looping per pixel.
    const unsigned char pixel_bytes[4] = {
        (unsigned char)(color.r * 255.0f),
        (unsigned char)(color.g * 255.0f),
        (unsigned char)(color.b * 255.0f),
        (unsigned char)(color.a * 255.0f)
    };
    u32 pattern;
    memcpy(&pattern, pixel_bytes, sizeof(pattern));
    ska_simd_fill_u32((u32*)texture->data, pattern, (usize)width * (usize)height);

    ska_texture_generate(texture);

    return texture;
}

SkaTexture* ska_texture_create_checkerboard(GLsizei width, GLsizei height) {
    SkaTexture* texture = ska_texture_create_default_texture();
    texture->nr_channels = 4;
    texture->width = width;
    texture->height = height;
    const GLsizei data_size = width * height * 4;
    texture->data = (unsigned char *)SKA_ALLOC_BYTES(data_size);

    // The color is constant within each 8-pixel-aligned run, so fill 8-pixel (or shorter,
    // at the right edge) runs at a time via SIMD instead of writing one pixel at a time.
    const unsigned char low_bytes[4] = {100, 100, 100, 255};
    const unsigned char high_bytes[4] = {200, 200, 200, 255};
    u32 pattern_low, pattern_high;
    memcpy(&pattern_low, low_bytes, sizeof(pattern_low));
    memcpy(&pattern_high, high_bytes, sizeof(pattern_high));

    for (int y = 0; y < height; y++) {
        u32* row = (u32*)(texture->data + (usize)y * (usize)width * 4);
        const int row_band = (y / 8) % 2;
        for (int x = 0; x < width; x += 8) {
            const int block = (x / 8 + row_band) % 2;
            const usize run_len = (usize)((x + 8 <= width) ? 8 : (width - x));
            ska_simd_fill_u32(row + x, block ? pattern_high : pattern_low, run_len);
        }
    }

    ska_texture_generate(texture);

    return texture;
}

SkaTexture* ska_texture_create_circle(GLsizei diameter) {
    SkaTexture* texture = ska_texture_create_default_texture();
    texture->nr_channels = 4;
    texture->width = diameter;
    texture->height = diameter;
    // Sampled at arbitrary scale via dest_size, so the antialiased edge below needs true bilinear
    // sampling, not the pixel-art nearest-neighbor snapping the sprite shader otherwise applies.
    texture->apply_nearest_neighbor = false;
    const GLsizei data_size = diameter * diameter * 4;
    texture->data = (unsigned char*)SKA_ALLOC_BYTES(data_size);

    const f32 radius = (f32)diameter / 2.0f;
    for (int y = 0; y < diameter; y++) {
        unsigned char* row = texture->data + (usize)y * (usize)diameter * 4;
        const f32 dy = ((f32)y + 0.5f) - radius;
        for (int x = 0; x < diameter; x++) {
            const f32 dx = ((f32)x + 0.5f) - radius;
            const f32 dist = sqrtf(dx * dx + dy * dy);
            // ~1px antialiased falloff at the edge instead of a hard cutoff.
            const f32 alpha = ska_math_clamp_float(radius - dist + 0.5f, 0.0f, 1.0f);
            unsigned char* pixel = row + (usize)x * 4;
            pixel[0] = 255;
            pixel[1] = 255;
            pixel[2] = 255;
            pixel[3] = (unsigned char)(alpha * 255.0f);
        }
    }

    ska_texture_generate(texture);

    return texture;
}

void ska_texture_generate(SkaTexture* texture) {
    // OpenGL Stuff
    if (texture->nr_channels == 1) {
      texture->image_format = GL_RED;
    } else if (texture->nr_channels == 3) {
      texture->image_format = GL_RGB;
    } else if (texture->nr_channels == 4) {
      texture->image_format = GL_RGBA;
    }

    // Create texture
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexImage2D(GL_TEXTURE_2D, 0, texture->internal_format, texture->width, texture->height, 0, texture->image_format, GL_UNSIGNED_BYTE, texture->data);
    glGenerateMipmap(GL_TEXTURE_2D);
    // Wrap and filter modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture->wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture->wrap_t);
    // Defaults to bilinear interpolation
    // TODO: Add min and mag params to texture
    const GLint min_filter = GL_LINEAR_MIPMAP_LINEAR;
    const GLint mag_filter = GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ska_texture_delete(SkaTexture* texture) {
    SKA_FREE(texture->file_name);
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
    return texture->wrap_s == GL_REPEAT ? "repeat" : "clamp_to_border";
}

const char* ska_texture_get_wrap_t_string(const SkaTexture* texture) {
    return texture->wrap_t == GL_REPEAT ? "repeat" : "clamp_to_border";
}

#endif // #if SKA_RENDERING
