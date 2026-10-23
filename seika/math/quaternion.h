#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "seika/defines.h"
#include "seika/math/math.h"

// --- SkaQuat --- //
typedef struct SkaQuat {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} SkaQuat;

#define SKA_QUAT_IDENTITY SKA_STRUCT_LITERAL(SkaQuat){ .x = 0.0f, .y = 0.0f, .z = 0.0f, .w = 1.0f }

// Construction
SkaQuat ska_quat_identity(void);
SkaQuat ska_quat_from_axis_angle(SkaVector3 axis, f32 angle_rad);
// Applies in Z -> Y -> X order, matching ska_matrix4_rotate_euler's composition order
SkaQuat ska_quat_from_euler(f32 x_angle_rad, f32 y_angle_rad, f32 z_angle_rad);
SkaQuat ska_quat_from_matrix4(const SkaMatrix4* matrix);

// Core ops
SkaQuat ska_quat_add(SkaQuat a, SkaQuat b);
SkaQuat ska_quat_sub(SkaQuat a, SkaQuat b);
SkaQuat ska_quat_scale(SkaQuat q, f32 scalar);
// Hamilton product / compose rotations: result rotates by 'b' first, then 'a' (i.e. result * v == a * (b * v))
SkaQuat ska_quat_mul(SkaQuat a, SkaQuat b);
f32 ska_quat_dot(SkaQuat a, SkaQuat b);
f32 ska_quat_length(SkaQuat q);
// Returns SKA_QUAT_IDENTITY on zero-length input (matches ska_vector3_normalize's degenerate-case convention)
SkaQuat ska_quat_normalize(SkaQuat q);
SkaQuat ska_quat_conjugate(SkaQuat q);
// Normalized conjugate; returns SKA_QUAT_IDENTITY on zero-length input
SkaQuat ska_quat_inverse(SkaQuat q);

// Interpolation
SkaQuat ska_quat_lerp(SkaQuat a, SkaQuat b, f32 alpha);
// Shortest-path spherical interpolation; falls back to lerp when inputs are nearly parallel
SkaQuat ska_quat_slerp(SkaQuat a, SkaQuat b, f32 alpha);

// Conversions
SkaMatrix4 ska_quat_to_matrix4(SkaQuat q);
// Radians, matching ska_matrix4_rotate_euler's parameter convention (NOT degrees, unlike SkaTransform2D.rotation)
SkaVector3 ska_quat_to_euler(SkaQuat q);
SkaVector3 ska_quat_rotate_vector3(SkaQuat q, SkaVector3 v);

// Equality
bool ska_quat_equals(SkaQuat a, SkaQuat b);
bool ska_quat_is_almost_equal(SkaQuat a, SkaQuat b, f32 epsilon);

#ifdef __cplusplus
}
#endif
