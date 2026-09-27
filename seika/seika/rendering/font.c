#include "font.h"

#if SKA_RENDERING

#include <ft2build.h>
#include FT_FREETYPE_H

#include "render_context.h"
#include "../logger.h"
#include "seika/memory.h"
#include "seika/asset/asset_file_loader.h"
#include "default_font_data.h"

static bool ska_generate_new_font_face(const char* file_name, FT_Face* face);
static void ska_initialize_font(FT_Face face, SkaFont* font, bool apply_nearest_neighbor);

SkaFont* ska_font_create_font(const char* file_name, int32 size, bool apply_nearest_neighbor) {
    FT_Face face;
    SkaFont* font = SKA_ALLOC(SkaFont);
    font->size = size;

    // Failed to create font, exit out early
    if (!ska_generate_new_font_face(file_name, &face)) {
        ska_logger_error("Freetype failed to load font '%s' with size '%d'!", file_name, size);
        font->is_valid = false;
        FT_Done_Face(face);
        return font;
    }

    ska_initialize_font(face, font, apply_nearest_neighbor);

    return font;
}

SkaFont* ska_font_create_font_from_memory(const void* buffer, usize buffer_size, int32 size, bool apply_nearest_neighbor) {
    FT_Face face;
    SkaFont* font = SKA_ALLOC(SkaFont);
    font->size = size;

    // Failed to create font, exit out early
    if (FT_New_Memory_Face(ska_render_context_get()->free_type_library,
                           (unsigned char *)buffer, (FT_Long)buffer_size, 0,
                           &face)) {
      ska_logger_error("Freetype failed to load font from memory!!");
      font->is_valid = false;
      FT_Done_Face(face);
      return font;
    }

    ska_initialize_font(face, font, apply_nearest_neighbor);

    return font;
}

SkaFont* ska_font_create_default_font(int32 size, bool apply_nearest_neighbor) {
    return ska_font_create_font_from_memory(ska_default_font_ttf_data, ska_default_font_ttf_data_size, size, apply_nearest_neighbor);
}

void ska_font_delete(SkaFont* font) {
    SKA_FREE(font);
}

bool ska_generate_new_font_face(const char* file_name, FT_Face* face) {
    if (ska_asset_file_loader_get_read_mode() == SkaAssetFileLoaderReadMode_ARCHIVE) {
        SkaArchiveFileAsset file_asset = ska_asset_file_loader_get_asset(file_name);
        if (ska_asset_file_loader_is_asset_valid(&file_asset)) {
          if (FT_New_Memory_Face(ska_render_context_get()->free_type_library,
                                 (unsigned char *)file_asset.buffer,
                                 (FT_Long)file_asset.buffer_size, 0, face)) {
            // Failed to create new face
            return false;
          }
        } else {
            // Failed to load asset from archive
            return false;
        }
    } else if (ska_asset_file_loader_get_read_mode() == SkaAssetFileLoaderReadMode_DISK) {
      if (FT_New_Face(ska_render_context_get()->free_type_library, file_name, 0,
                      face)) {
        // Failed to create new face
        return false;
      }
    }
    return true;
}

static void ska_initialize_font(FT_Face face, SkaFont* font, bool apply_nearest_neighbor) {
    // Set size to load glyphs, width set to 0 to dynamically adjust
    FT_Set_Pixel_Sizes(face, 0, font->size);
    font->ascent = (f32) (face->size->metrics.ascender >> 6);
    // Disable byte alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // Load first 128 characters of ASCII set
    for (unsigned char c = 0; c < 128; c++) { // 'c++' ;-)
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            ska_logger_error("Freetype failed to load glyph '%u!", c);
            continue;
        }
        // Generate texture
        GLuint text_texture;
        glGenTextures(1, &text_texture);
        glBindTexture(GL_TEXTURE_2D, text_texture);
        // GL_R8 (sized), not GL_RED (unsized), as internalformat -- GLES3/WebGL2 rejects GL_RED
        // there outright ("Invalid internalformat"), even though format=GL_RED matches; GL_R8 is
        // the portable single-channel choice and is equally valid on desktop GL.
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_R8,
            (GLsizei) face->glyph->bitmap.width,
            (GLsizei) face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // Texture wrap and filter options
        const GLint filter_type = apply_nearest_neighbor ? GL_NEAREST : GL_LINEAR;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_type);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_type);
        // Create character struct
        SkaFontCharacter character = {.texture_id = text_texture,
                                      .size = {(f32)face->glyph->bitmap.width,
                                               (f32)face->glyph->bitmap.rows},
                                      .bearing = {(f32)face->glyph->bitmap_left,
                                                  (f32)face->glyph->bitmap_top},
                                      .advance =
                                          (GLuint)face->glyph->advance.x};
        font->characters[c] = character;
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    // configure VAO & VBO texture quads
    glGenVertexArrays(1, &font->vao);
    glGenBuffers(1, &font->vbo);
    glBindVertexArray(font->vao);
    glBindBuffer(GL_ARRAY_BUFFER, font->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    font->is_valid = true;

    FT_Done_Face(face);
}

#endif // #if SKA_RENDERING
