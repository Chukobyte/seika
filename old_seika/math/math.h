#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#define CGLM_ALL_UNALIGNED
#endif

#include <cglm/cglm.h>

#include "seika/defines.h"

#define SKA_PI 3.14159265358979323846f
#define SKA_RAD_2_DEG(RADIANS) ((RADIANS) * (180.0 / (f64)SKA_PI))
#define SKA_RAD_2_DEGF(RADIANS) ((RADIANS) * (180.0f / SKA_PI))
#define SKA_DEG_2_RAD(DEGREES) ((DEGREES) * ((f64)SKA_PI / 180.0))
#define SKA_DEG_2_RADF(DEGREES) ((DEGREES) * (SKA_PI / 180.0f))


// --- SkaVector2 --- //
typedef struct SkaVector2 {
    f32 x;
    f32 y;
} SkaVector2;

#define SKA_VECTOR2_ZERO SKA_STRUCT_LITERAL(SkaVector2){ 0.0f, 0.0f }
#define SKA_VECTOR2_ONE SKA_STRUCT_LITERAL(SkaVector2){ 1.0f, 1.0f }
#define SKA_VECTOR2_LEFT SKA_STRUCT_LITERAL(SkaVector2){ -1.0f, 0.0f }
#define SKA_VECTOR2_RIGHT SKA_STRUCT_LITERAL(SkaVector2){ 1.0f, 0.0f }
#define SKA_VECTOR2_UP SKA_STRUCT_LITERAL(SkaVector2){ 0.0f, -1.0f }
#define SKA_VECTOR2_DOWN SKA_STRUCT_LITERAL(SkaVector2){ 0.0f, 1.0f }

bool ska_math_vec2_equals(const SkaVector2* v1, const SkaVector2* v2);
SkaVector2 ska_math_vec2_lerp(const SkaVector2* v1, const SkaVector2* v2, f32 alpha);
f32 ska_math_vec2_angle(const SkaVector2* v);

// --- SkaVector2i --- //
typedef struct SkaVector2i {
    int32 x;
    int32 y;
} SkaVector2i;

#define SKA_VECTOR2I_ZERO SKA_STRUCT_LITERAL(SkaVector2i){ 0, 0 }
#define SKA_VECTOR2I_ONE SKA_STRUCT_LITERAL(SkaVector2i){ 1, 1 }
#define SKA_VECTOR2I_LEFT SKA_STRUCT_LITERAL(SkaVector2i){ -1, 0 }
#define SKA_VECTOR2I_RIGHT SKA_STRUCT_LITERAL(SkaVector2i){ 1, 0 }
#define SKA_VECTOR2I_UP SKA_STRUCT_LITERAL(SkaVector2i){ 0, -1 }
#define SKA_VECTOR2I_DOWN SKA_STRUCT_LITERAL(SkaVector2i){ 0, 1 }

// --- SkaSize2D --- //
typedef struct SkaSize2D {
    f32 w;
    f32 h;
} SkaSize2D;

#define SKA_SIZE2D_ZERO SKA_STRUCT_LITERAL(SkaSize2D){ 0.0f, 0.0f }

// --- SkaSize2Di --- //
typedef struct SkaSize2Di {
    int32 w;
    int32 h;
} SkaSize2Di;

#define SKA_SIZE2DI_ZERO SKA_STRUCT_LITERAL(SkaSize2Di){ 0, 0 }

// --- SkaRect2 --- //
typedef struct SkaRect2 {
    f32 x;
    f32 y;
    f32 w;
    f32 h;
} SkaRect2;

#define SKA_RECT2D_ZERO SKA_STRUCT_LITERAL(SkaRect2){ 0.0f, 0.0f, 0.0f, 0.0f }

bool se_rect2_does_rectangles_overlap(const SkaRect2* sourceRect, const SkaRect2* targetRect);

//--- SkaTransform2D ---//
typedef struct SkaTransform2D {
    SkaVector2 position;
    SkaVector2 scale;
    f32 rotation; // degrees
} SkaTransform2D;

void ska_transform2d_mat4_to_transform(mat4 matrix, SkaTransform2D* transform);
void ska_transform2d_transform_to_mat4(const SkaTransform2D* transform, mat4 matrix);
SkaTransform2D ska_transform2d_lerp(const SkaTransform2D* tA, const SkaTransform2D* tB, f32 alpha);

#define SKA_TRANSFORM_IDENTITY SKA_STRUCT_LITERAL(SkaTransform2D){ \
    .position = SKA_VECTOR2_ZERO, \
    .scale = SKA_VECTOR2_ONE, \
    .rotation = 0.0f \
}

typedef struct SkaTransformModel2D {
    SkaVector2 position;
    SkaVector2 scale;
    f32 rotation; // degrees
    int32 zIndex;
    SkaVector2 scaleSign;
    mat4 model;
} SkaTransformModel2D;

SkaTransform2D ska_transform2d_model_convert_to_transform(SkaTransformModel2D* transformModel2D);

#define SKA_TRANSFORM_MODEL_IDENTITY SKA_STRUCT_LITERAL(SkaTransformModel2D){ \
    .position = SKA_VECTOR2_ZERO, \
    .scale = SKA_VECTOR2_ONE, \
    .rotation = 0.0f, \
    .zIndex = 0, \
    .scaleSign = SKA_VECTOR2_ONE \
}

// --- SkaVector3 --- //
typedef struct SkaVector3 {
    f32 x;
    f32 y;
    f32 z;
} SkaVector3;

// --- SkaVector4 --- //
typedef struct SkaVector4 {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} SkaVector4;

// --- SkaColor --- //
typedef struct SkaColor {
    f32 r;
    f32 g;
    f32 b;
    f32 a;
} SkaColor;

#define SKA_COLOR_WHITE SKA_STRUCT_LITERAL(SkaColor){ 1.0f, 1.0f, 1.0f, 1.0f }
#define SKA_COLOR_BLACK SKA_STRUCT_LITERAL(SkaColor){ 0.0f, 0.0f, 0.0f, 1.0f }
#define SKA_COLOR_RED SKA_STRUCT_LITERAL(SkaColor){ 1.0f, 0.0f, 0.0f, 1.0f }
#define SKA_COLOR_GREEN SKA_STRUCT_LITERAL(SkaColor){ 0.0f, 1.0f, 0.0f, 1.0f }
#define SKA_COLOR_BLUE SKA_STRUCT_LITERAL(SkaColor){ 0.0f, 0.0f, 1.0f, 1.0f }

SkaColor ska_color_get_normalized_color_default_alpha(uint32 r, uint32 g, uint32 b);
SkaColor ska_color_get_normalized_color(uint32 r, uint32 g, uint32 b, uint32 a);
SkaColor ska_color_get_normalized_color_from_color(const SkaColor* color);

// --- SkaMinMaxVec2 --- //
typedef struct SkaMinMaxVec2 {
    SkaVector2 min;
    SkaVector2 max;
} SkaMinMaxVec2;

#define SKA_MINMAX_VEC2_ZERO SKA_STRUCT_LITERAL(SkaMinMaxVec2){ SKA_VECTOR2_ZERO, SKA_VECTOR2_ZERO }

SkaVector2 ska_math_minmax_vec2_get_random_in_range(const SkaMinMaxVec2* minmax);

// --- Misc --- //
#define SKA_MATH_MIN(A, B) ((A) < (B) ? (A) : (B))
#define SKA_MATH_MAX(A, B) ((A) > (B) ? (A) : (B))
#define SKA_MATH_CLAMP(VAL, MIN_VAL, MAX_VAL) ((VAL) < (MIN_VAL) ? (MIN_VAL) : (VAL) > (MAX_VAL) ? (MAX_VAL) : (VAL))

f32 ska_math_lerpf(f32 a, f32 b, f32 alpha);
f32 ska_math_map_to_range(f32 input, f32 inputMin, f32 inputMax, f32 outputMin, f32 outputMax);
f32 ska_math_map_to_unit(f32 input, f32 inputMin, f32 inputMax);
f64 ska_math_map_to_range_double(f64 input, f64 inputMin, f64 inputMax, f64 outputMin, f64 outputMax);
f64 ska_math_map_to_unit_double(f64 input, f64 inputMin, f64 inputMax);
f32 ska_math_signf(f32 value);
SkaVector2 ska_math_signvec2(SkaVector2* value);
int32 ska_math_clamp_int(int32 value, int32 min, int32 max);
f32 ska_math_clamp_float(f32 value, f32 min, f32 max);
bool ska_math_is_almost_equal_float(f32 v1, f32 v2, f32 epsilon);
bool ska_math_is_almost_equal_float_default(f32 v1, f32 v2);
bool ska_math_is_almost_equal_double(f64 v1, f64 v2, f64 epsilon);
bool ska_math_is_almost_equal_double_default(f64 v1, f64 v2);

#ifdef __cplusplus
}
#endif
