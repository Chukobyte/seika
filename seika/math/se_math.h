#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#define CGLM_ALL_UNALIGNED
#endif

#include <cglm/cglm.h>

#define SKA_PI 3.14159265358979323846f
#define SKA_RAD_2_DEG (180.0f / SKA_PI)

// --- SKAVector2 --- //
typedef struct SKAVector2 {
    float x;
    float y;
} SKAVector2;

#define SKA_VECTOR2_ZERO (SKAVector2){ 0.0f, 0.0f }
#define SKA_VECTOR2_ONE (SKAVector2){ 1.0f, 1.0f }
#define SKA_VECTOR2_LEFT (SKAVector2){ -1.0f, 0.0f }
#define SKA_VECTOR2_RIGHT (SKAVector2){ 1.0f, 0.0f }
#define SKA_VECTOR2_UP (SKAVector2){ 0.0f, -1.0f }
#define SKA_VECTOR2_DOWN (SKAVector2){ 0.0f, 1.0f }

bool ska_math_vec2_equals(const SKAVector2* v1, const SKAVector2* v2);
SKAVector2 ska_math_vec2_lerp(const SKAVector2* v1, const SKAVector2* v2, float t);

// --- SKAVector2i --- //
typedef struct SKAVector2i {
    int x;
    int y;
} SKAVector2i;

// --- SKASize2D --- //
typedef struct SKASize2D {
    float w;
    float h;
} SKASize2D;

#define SKA_SIZE2D_ZERO (SKASize2D){ 0.0f, 0.0f }

// --- SKASize2Di --- //
typedef struct SKASize2Di {
    int w;
    int h;
} SKASize2Di;

// --- SKARect2 --- //
typedef struct SKARect2 {
    float x;
    float y;
    float w;
    float h;
} SKARect2;

#define SKA_RECT2D_ZERO (SKARect2){ 0.0f, 0.0f, 0.0f, 0.0f }

bool se_rect2_does_rectangles_overlap(const SKARect2* sourceRect, const SKARect2* targetRect);

//--- SKATransform2D ---//
typedef struct SKATransform2D {
    SKAVector2 position;
    SKAVector2 scale;
    float rotation; // degrees
} SKATransform2D;

#define SKA_TRANSFORM_IDENTITY (SKATransform2D){ \
    .position = SKA_VECTOR2_ZERO, \
    .scale = SKA_VECTOR2_ONE, \
    .rotation = 0.0f \
}

typedef struct SKATransformModel2D {
    SKAVector2 position;
    SKAVector2 scale;
    float rotation; // degrees
    int zIndex;
    SKAVector2 scaleSign;
    mat4 model;
} SKATransformModel2D;

#define SKA_TRANSFORM_MODEL_IDENTITY (SKATransformModel2D){ \
    .position = SKA_VECTOR2_ZERO, \
    .scale = SKA_VECTOR2_ONE, \
    .rotation = 0.0f, \
    .zIndex = 0, \
    .scaleSign = SKA_VECTOR2_ONE \
}

// --- SKAVector3 --- //
typedef struct SKAVector3 {
    float x;
    float y;
    float z;
} SKAVector3;

// --- SKAVector4 --- //
typedef struct SKAVector4 {
    float x;
    float y;
    float z;
    float w;
} SKAVector4;

// --- SKAColor --- //
typedef struct SKAColor {
    float r;
    float g;
    float b;
    float a;
} SKAColor;

#define SKA_COLOR_WHITE (SKAColor){ 1.0f, 1.0f, 1.0f, 1.0f }
#define SKA_COLOR_BLACK (SKAColor){ 0.0f, 0.0f, 0.0f, 1.0f }
#define SKA_COLOR_RED (SKAColor){ 1.0f, 0.0f, 0.0f, 1.0f }
#define SKA_COLOR_GREEN (SKAColor){ 0.0f, 1.0f, 0.0f, 1.0f }
#define SKA_COLOR_BLUE (SKAColor){ 0.0f, 0.0f, 1.0f, 1.0f }

SKAColor ska_color_get_normalized_color_default_alpha(unsigned int r, unsigned int g, unsigned int b);
SKAColor ska_color_get_normalized_color(unsigned int r, unsigned int g, unsigned int b, unsigned int a);
SKAColor ska_color_get_normalized_color_from_color(const SKAColor* color);

// --- Misc --- //
float ska_math_lerpf(float a, float b, float t);
float ska_math_map_to_range(float input, float inputMin, float inputMax, float outputMin, float outputMax);
float ska_math_map_to_unit(float input, float inputMin, float inputMax);
double ska_math_map_to_range_double(double input, double inputMin, double inputMax, double outputMin, double outputMax);
double ska_math_map_to_unit_double(double input, double inputMin, double inputMax);
float ska_math_signf(float value);
SKAVector2 ska_math_signvec2(SKAVector2* value);
int ska_math_clamp_int(int value, int min, int max);
float ska_math_clamp_float(float value, float min, float max);
bool ska_math_is_almost_equal_float(float v1, float v2, float epsilon);
bool ska_math_is_almost_equal_float_default(float v1, float v2);
bool ska_math_is_almost_equal_double(double v1, double v2, double epsilon);
bool ska_math_is_almost_equal_double_default(double v1, double v2);

#ifdef __cplusplus
}
#endif
