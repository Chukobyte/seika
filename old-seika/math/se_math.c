#include "se_math.h"

// --- Vector2 --- //
bool ska_math_vec2_equals(const SKAVector2* v1, const SKAVector2* v2) {
    return v1->x == v2->x && v1->y == v2->y;
}

SKAVector2 ska_math_vec2_lerp(const SKAVector2* v1, const SKAVector2* v2, float alpha) {
    return (SKAVector2) {
        .x = ska_math_lerpf(v1->x, v2->x, alpha),
        .y = ska_math_lerpf(v1->y, v2->y, alpha)
    };
}

float ska_math_vec2_angle(const SKAVector2* v) {
    float angle = atan2f(v->y, v->x);
    if (angle < 0.0f) {
        angle += 2.0f * SKA_PI;
    }
    return angle;
}

// --- Rect2 --- //
bool se_rect2_does_rectangles_overlap(const SKARect2* sourceRect, const SKARect2* targetRect) {
    return (sourceRect->x + sourceRect->w >= targetRect->x) &&
           (targetRect->x + targetRect->w >= sourceRect->x) &&
           (sourceRect->y + sourceRect->h >= targetRect->y) &&
           (targetRect->y + targetRect->h >= sourceRect->y);
}

// --- Transform2D --- //
void ska_transform2d_mat4_to_transform(mat4 matrix, SKATransform2D* transform) {
    // Decompose trs matrix
    vec4 translation;
    mat4 rotation;
    vec3 scale;
    glm_decompose(matrix, translation, rotation, scale);
    // Update position
    transform->position = (SKAVector2){ .x = translation[0], .y = translation[1] };
    // Update scale
    transform->scale = (SKAVector2){ .x = fabsf(scale[0]), .y = fabsf(scale[1]) };
    // Convert rotation to degrees
    versor quat;
    glm_mat4_quat(rotation, quat);
    const float angleRadians = glm_quat_angle(quat);
    transform->rotation = glm_deg(angleRadians);
}

void ska_transform2d_transform_to_mat4(const SKATransform2D* transform, mat4 matrix) {
    // Create translation matrix
    glm_translate_make(matrix, (vec3){transform->position.x, transform->position.y, 0.0f});
    // Create rotation matrix (convert degrees to radians)
    glm_rotate_z(matrix, glm_rad(transform->rotation), matrix);
    // Create scale matrix
    glm_scale(matrix, (vec3){transform->scale.x, transform->scale.y, 1.0f});
}

SKATransform2D ska_transform2d_lerp(const SKATransform2D* tA, const SKATransform2D* tB, float alpha) {
    return (SKATransform2D) {
            .position = ska_math_vec2_lerp(&tA->position, &tB->position, alpha),
            .scale = ska_math_vec2_lerp(&tA->scale, &tB->scale, alpha),
            .rotation = ska_math_lerpf(tA->rotation, tB->rotation, alpha)
    };
}

// --- Transform2D Model --- //
SKATransform2D ska_transform2d_model_convert_to_transform(SKATransformModel2D* transformModel2D) {
    SKATransform2D transform2D;
    ska_transform2d_mat4_to_transform(transformModel2D->model, &transform2D);
    // Update scale sign
    transform2D.scale = (SKAVector2){ .x = transform2D.scale.x * transformModel2D->scaleSign.x, .y = transform2D.scale.y * transformModel2D->scaleSign.y };
    return transform2D;
}

// --- Color --- //
SKAColor ska_color_get_normalized_color_default_alpha(unsigned int r, unsigned int g, unsigned int b) {
    SKAColor color = {
        .r = (float) r / 255.0f,
        .g = (float) g / 255.0f,
        .b = (float) b / 255.0f,
        .a = 1.0f
    };
    return color;
}

SKAColor ska_color_get_normalized_color(unsigned int r, unsigned int g, unsigned int b, unsigned int a) {
    SKAColor color = {
        .r = (float) r / 255.0f,
        .g = (float) g / 255.0f,
        .b = (float) b / 255.0f,
        .a = (float) a / 255.0f
    };
    return color;
}

SKAColor ska_color_get_normalized_color_from_color(const SKAColor* color) {
    SKAColor newColor = {
        .r = color->r / 255.0f,
        .g = color->g / 255.0f,
        .b = color->b / 255.0f,
        .a = color->a / 255.0f
    };
    return newColor;
}

SKAVector2 ska_math_minmax_vec2_get_random_in_range(const SKAMinMaxVec2* minmax) {
    const SKAVector2 randomVector = {
        .x = minmax->min.x + (float)rand() / RAND_MAX * (minmax->max.x - minmax->min.x),
        .y = minmax->min.y + (float)rand() / RAND_MAX * (minmax->max.y - minmax->min.y),
    };
    return randomVector;
}

// --- Misc --- //
float ska_math_lerpf(float a, float b, float alpha) {
    return a + (b - a) * alpha;
}

float ska_math_map_to_range(float input, float inputMin, float inputMax, float outputMin, float outputMax) {
    return (((input - inputMin) / (inputMax - inputMin)) * (outputMax - outputMin) + outputMin);
}

float ska_math_map_to_unit(float input, float inputMin, float inputMax) {
    return ska_math_map_to_range(input, inputMin, inputMax, 0.0f, 1.0f);
}

double ska_math_map_to_range_double(double input, double inputMin, double inputMax, double outputMin, double outputMax) {
    return (((input - inputMin) / (inputMax - inputMin)) * (outputMax - outputMin) + outputMin);
}

double ska_math_map_to_unit_double(double input, double inputMin, double inputMax) {
    return ska_math_map_to_range_double(input, inputMin, inputMax, 0.0, 1.0);
}

float ska_math_signf(float value) {
    if (value > 0.0f) {
        return 1.0f;
    } else if(value < 0.0f) {
        return -1.0f;
    }
    return 0.0f;
}

SKAVector2 ska_math_signvec2(SKAVector2* value) {
    SKAVector2 sign_vec = {
        .x = ska_math_signf(value->x),
        .y = ska_math_signf(value->y)
    };
    return sign_vec;
}

int ska_math_clamp_int(int value, int min, int max) {
    return value < min ? min : (value > max ? max : value);
}

float ska_math_clamp_float(float value, float min, float max) {
    return value < min ? min : (value > max ? max : value);
}

bool ska_math_is_almost_equal_float(float v1, float v2, float epsilon) {
    return fabsf(v1 - v2) <= epsilon;
}

bool ska_math_is_almost_equal_float_default(float v1, float v2) {
    static const double epsilon = 0.001f;
    return fabsf(v1 - v2) <= epsilon;
}

bool ska_math_is_almost_equal_double(double v1, double v2, double epsilon) {
    return fabs(v1 - v2) <= epsilon;
}

bool ska_math_is_almost_equal_double_default(double v1, double v2) {
    static const double epsilon = 0.001;
    return fabs(v1 - v2) <= epsilon;
}
