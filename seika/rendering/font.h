#pragma once

#if SKA_RENDERING

#include <stdbool.h>

#include "seika/rendering/ska_opengl.h"
#include "seika/math/math.h"

typedef struct SkaFontCharacter {
  GLuint texture_id;
  SkaVector2 size;
  SkaVector2 bearing;
  uint32 advance;
} SkaFontCharacter;

typedef struct SkaFont {
  bool is_valid;
  GLuint vao;
  GLuint vbo;
  int32 size;
  f32 ascent;
  SkaFontCharacter characters[128]; // First 128 of ASCII set
} SkaFont;

SkaFont *ska_font_create_font(const char *file_name, int32 size,
                              bool apply_nearest_neighbor);
SkaFont *ska_font_create_font_from_memory(const void *buffer, usize buffer_size,
                                          int32 size,
                                          bool apply_nearest_neighbor);
// Loads seika's bundled fallback font (Roboto Mono, SIL OFL 1.1) from memory, so callers don't
// need to source or ship a .ttf, or depend on a platform-specific system font path.
SkaFont *ska_font_create_default_font(int32 size, bool apply_nearest_neighbor);
void ska_font_delete(SkaFont* font);
// Glyph for one byte of UTF-8 text: ASCII maps directly, a multi-byte character's lead byte
// gets '?' (only the first 128 glyphs are loaded), continuation bytes return NULL (skip them).
const SkaFontCharacter* ska_font_get_glyph(const SkaFont* font, unsigned char byte);

#endif // #if SKA_RENDERING