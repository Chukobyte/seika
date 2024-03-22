#include "math.h"

// --- Vector2 --- //
bool ska_math_vec2_equals(const SkaVector2* v1, const SkaVector2* v2) {
    return v1->x == v2->x && v1->y == v2->y;
}

SkaVector2 ska_math_vec2_lerp(const SkaVector2* v1, const SkaVector2* v2, f32 alpha) {
    return (SkaVector2) {
            .x = ska_math_lerpf(v1->x, v2->x, alpha),
            .y = ska_math_lerpf(v1->y, v2->y, alpha)
    };
}

f32 ska_math_vec2_angle(const SkaVector2* v) {
    f32 angle = atan2f(v->y, v->x);
    if (angle < 0.0f) {
        angle += 2.0f * SKA_PI;
    }
    return angle;
}

// --- Rect2 --- //
bool se_rect2_does_rectangles_overlap(const SkaRect2* sourceRect, const SkaRect2* targetRect) {
    return (sourceRect->x + sourceRect->w >= targetRect->x) &&
           (targetRect->x + targetRect->w >= sourceRect->x) &&
           (sourceRect->y + sourceRect->h >= targetRect->y) &&
           (targetRect->y + targetRect->h >= sourceRect->y);
}

// --- Transform2D --- //
void ska_transform2d_mat4_to_transform(mat4 matrix, SkaTransform2D* transform) {
    // Decompose trs matrix
    vec4 translation;
    mat4 rotation;
    vec3 scale;
    glm_decompose(matrix, translation, rotation, scale);
    // Update position
    transform->position = (SkaVector2){ .x = translation[0], .y = translation[1] };
    // Update scale
    transform->scale = (SkaVector2){ .x = fabsf(scale[0]), .y = fabsf(scale[1]) };
    // Convert rotation to degrees
    versor quat;
    glm_mat4_quat(rotation, quat);
    const f32 angleRadians = glm_quat_angle(quat);
    transform->rotation = glm_deg(angleRadians);
}

void ska_transform2d_transform_to_mat4(const SkaTransform2D* transform, mat4 matrix) {
    // Create translation matrix
    glm_translate_make(matrix, (vec3){transform->position.x, transform->position.y, 0.0f});
    // Create rotation matrix (convert degrees to radians)
    glm_rotate_z(matrix, glm_rad(transform->rotation), matrix);
    // Create scale matrix
    glm_scale(matrix, (vec3){transform->scale.x, transform->scale.y, 1.0f});
}

SkaTransform2D ska_transform2d_lerp(const SkaTransform2D* tA, const SkaTransform2D* tB, f32 alpha) {
    return (SkaTransform2D) {
            .position = ska_math_vec2_lerp(&tA->position, &tB->position, alpha),
            .scale = ska_math_vec2_lerp(&tA->scale, &tB->scale, alpha),
            .rotation = ska_math_lerpf(tA->rotation, tB->rotation, alpha)
    };
}

// --- Transform2D Model --- //
SkaTransform2D ska_transform2d_model_convert_to_transform(SkaTransformModel2D* transformModel2D) {
    SkaTransform2D transform2D;
    ska_transform2d_mat4_to_transform(transformModel2D->model, &transform2D);
    // Update scale sign
    transform2D.scale = (SkaVector2){ .x = transform2D.scale.x * transformModel2D->scaleSign.x, .y = transform2D.scale.y * transformModel2D->scaleSign.y };
    return transform2D;
}

// --- Color --- //
SkaColor ska_color_get_normalized_color_default_alpha(uint32 r, uint32 g, uint32 b) {
    SkaColor color = {
            .r = (f32) r / 255.0f,
            .g = (f32) g / 255.0f,
            .b = (f32) b / 255.0f,
            .a = 1.0f
    };
    return color;
}

SkaColor ska_color_get_normalized_color(uint32 r, uint32 g, uint32 b, uint32 a) {
    SkaColor color = {
            .r = (f32) r / 255.0f,
            .g = (f32) g / 255.0f,
            .b = (f32) b / 255.0f,
            .a = (f32) a / 255.0f
    };
    return color;
}

SkaColor ska_color_get_normalized_color_from_color(const SkaColor* color) {
    SkaColor newColor = {
            .r = color->r / 255.0f,
            .g = color->g / 255.0f,
            .b = color->b / 255.0f,
            .a = color->a / 255.0f
    };
    return newColor;
}

SkaVector2 ska_math_minmax_vec2_get_random_in_range(const SkaMinMaxVec2* minmax) {
    const SkaVector2 randomVector = {
            .x = minmax->min.x + (f32)rand() / RAND_MAX * (minmax->max.x - minmax->min.x),
            .y = minmax->min.y + (f32)rand() / RAND_MAX * (minmax->max.y - minmax->min.y),
    };
    return randomVector;
}

// --- Misc --- //
f32 ska_math_lerpf(f32 a, f32 b, f32 alpha) {
    return a + (b - a) * alpha;
}

f32 ska_math_map_to_range(f32 input, f32 inputMin, f32 inputMax, f32 outputMin, f32 outputMax) {
    return (((input - inputMin) / (inputMax - inputMin)) * (outputMax - outputMin) + outputMin);
}

f32 ska_math_map_to_unit(f32 input, f32 inputMin, f32 inputMax) {
    return ska_math_map_to_range(input, inputMin, inputMax, 0.0f, 1.0f);
}

f64 ska_math_map_to_range_double(f64 input, f64 inputMin, f64 inputMax, f64 outputMin, f64 outputMax) {
    return (((input - inputMin) / (inputMax - inputMin)) * (outputMax - outputMin) + outputMin);
}

f64 ska_math_map_to_unit_double(f64 input, f64 inputMin, f64 inputMax) {
    return ska_math_map_to_range_double(input, inputMin, inputMax, 0.0, 1.0);
}

f32 ska_math_signf(f32 value) {
    if (value > 0.0f) {
        return 1.0f;
    } else if(value < 0.0f) {
        return -1.0f;
    }
    return 0.0f;
}

SkaVector2 ska_math_signvec2(SkaVector2* value) {
    SkaVector2 sign_vec = {
            .x = ska_math_signf(value->x),
            .y = ska_math_signf(value->y)
    };
    return sign_vec;
}

int ska_math_clamp_int(int value, int min, int max) {
    return value < min ? min : (value > max ? max : value);
}

f32 ska_math_clamp_float(f32 value, f32 min, f32 max) {
    return value < min ? min : (value > max ? max : value);
}

bool ska_math_is_almost_equal_float(f32 v1, f32 v2, f32 epsilon) {
    return fabsf(v1 - v2) <= epsilon;
}

bool ska_math_is_almost_equal_float_default(f32 v1, f32 v2) {
    static const f64 epsilon = 0.001f;
    return fabsf(v1 - v2) <= epsilon;
}

bool ska_math_is_almost_equal_double(f64 v1, f64 v2, f64 epsilon) {
    return fabs(v1 - v2) <= epsilon;
}

bool ska_math_is_almost_equal_double_default(f64 v1, f64 v2) {
    static const f64 epsilon = 0.001;
    return fabs(v1 - v2) <= epsilon;
}
