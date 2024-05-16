#pragma once

#include <stdbool.h>

#include <glad/glad.h>

#include "seika/math/math.h"

typedef struct SkaFontCharacter {
    GLuint textureId;
    SkaVector2 size;
    SkaVector2 bearing;
    uint32 advance;
} SkaFontCharacter;

typedef struct SkaFont {
    bool isValid;
    GLuint VAO;
    GLuint VBO;
    int32 size;
    SkaFontCharacter characters[128]; // First 128 of ASCII set
} SkaFont;

SkaFont* ska_font_create_font(const char* fileName, int32 size, bool applyNearestNeighbor);
SkaFont* ska_font_create_font_from_memory(void* buffer, usize bufferSize, int32 size, bool applyNearestNeighbor);
void ska_font_delete(SkaFont* font);
