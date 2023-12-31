#pragma once

#include <stdbool.h>

#include <glad/glad.h>

#include "../math/se_math.h"

typedef struct SECharacter {
    GLuint textureId;
    SKAVector2 size;
    SKAVector2 bearing;
    unsigned int advance;
} SECharacter;

typedef struct SEFont {
    bool isValid;
    GLuint VAO;
    GLuint VBO;
    int size;
    SECharacter characters[128]; // First 128 of ASCII set
} SEFont;

SEFont* ska_font_create_font(const char* fileName, int size, bool applyNearestNeighbor);
SEFont* ska_font_create_font_from_memory(void* buffer, size_t bufferSize, int size, bool applyNearestNeighbor);
