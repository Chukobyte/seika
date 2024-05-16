#include "font.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "render_context.h"
#include "seika/logger.h"
#include "seika/memory.h"
#include "seika/asset/asset_file_loader.h"

static bool ska_generate_new_font_face(const char* fileName, FT_Face* face);
static void ska_initialize_font(FT_Face face, SkaFont* font, bool applyNearestNeighbor);

SkaFont* ska_font_create_font(const char* fileName, int32 size, bool applyNearestNeighbor) {
    FT_Face face;
    SkaFont* font = SKA_MEM_ALLOCATE(SkaFont);
    font->size = size;

    // Failed to create font, exit out early
    if (!ska_generate_new_font_face(fileName, &face)) {
        ska_logger_error("Freetype failed to load font '%s' with size '%d'!", fileName, size);
        font->isValid = false;
        FT_Done_Face(face);
        return font;
    }

    ska_initialize_font(face, font, applyNearestNeighbor);

    return font;
}

SkaFont* ska_font_create_font_from_memory(void* buffer, usize bufferSize, int32 size, bool applyNearestNeighbor) {
    FT_Face face;
    SkaFont* font = SKA_MEM_ALLOCATE(SkaFont);
    font->size = size;

    // Failed to create font, exit out early
    if(FT_New_Memory_Face(ska_render_context_get()->freeTypeLibrary, (unsigned char*)buffer, (FT_Long)bufferSize, 0, &face)) {
        ska_logger_error("Freetype failed to load font from memory!!");
        font->isValid = false;
        FT_Done_Face(face);
        return font;
    }

    ska_initialize_font(face, font, applyNearestNeighbor);

    return font;
}

void ska_font_delete(SkaFont* font) {
    SKA_MEM_FREE(font);
}

bool ska_generate_new_font_face(const char* fileName, FT_Face* face) {
    if (ska_asset_file_loader_get_read_mode() == SkaAssetFileLoaderReadMode_ARCHIVE) {
        SkaArchiveFileAsset fileAsset = ska_asset_file_loader_get_asset(fileName);
        if (ska_asset_file_loader_is_asset_valid(&fileAsset)) {
            if (FT_New_Memory_Face(ska_render_context_get()->freeTypeLibrary, (unsigned char*) fileAsset.buffer, (FT_Long) fileAsset.bufferSize, 0, face)) {
                // Failed to create new face
                return false;
            }
        } else {
            // Failed to load asset from archive
            return false;
        }
    } else if (ska_asset_file_loader_get_read_mode() == SkaAssetFileLoaderReadMode_DISK) {
        if (FT_New_Face(ska_render_context_get()->freeTypeLibrary, fileName, 0, face)) {
            // Failed to create new face
            return false;
        }
    }
    return true;
}

static void ska_initialize_font(FT_Face face, SkaFont* font, bool applyNearestNeighbor) {
    // Set size to load glyphs, width set to 0 to dynamically adjust
    FT_Set_Pixel_Sizes(face, 0, font->size);
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
        GLuint textTexture;
        glGenTextures(1, &textTexture);
        glBindTexture(GL_TEXTURE_2D, textTexture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            (GLsizei) face->glyph->bitmap.width,
            (GLsizei) face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // Texture wrap and filter options
        const GLint filterType = applyNearestNeighbor ? GL_NEAREST : GL_LINEAR;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterType);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterType);
        // Create character struct
        SkaFontCharacter character = {
                .textureId = textTexture,
                .size = { (f32)face->glyph->bitmap.width, (f32)face->glyph->bitmap.rows },
                .bearing = { (f32)face->glyph->bitmap_left, (f32)face->glyph->bitmap_top },
                .advance = (GLuint) face->glyph->advance.x
        };
        font->characters[c] = character;
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    // configure VAO & VBO texture quads
    glGenVertexArrays(1, &font->VAO);
    glGenBuffers(1, &font->VBO);
    glBindVertexArray(font->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, font->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    font->isValid = true;

    FT_Done_Face(face);
}
