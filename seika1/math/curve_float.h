#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

#include "seika/defines.h"

#define SKA_CURVE_MAX_CONTROL_POINTS 16

typedef struct SkaCurveControlPoint {
    f64 x; // a.k.a time
    f64 y; // a.k.a value
    f64 tangentIn;
    f64 tangentOut;
} SkaCurveControlPoint;

typedef struct SkaCurveFloat {
    usize controlPointCount;
    SkaCurveControlPoint controlPoints[SKA_CURVE_MAX_CONTROL_POINTS];
} SkaCurveFloat;

void ska_curve_float_add_control_point(SkaCurveFloat* curve, SkaCurveControlPoint point);
void ska_curve_float_add_control_points(SkaCurveFloat* curve, SkaCurveControlPoint points[], usize count);
bool ska_curve_float_remove_control_point(SkaCurveFloat* curve, f64 x, f64 y);
f64 ska_curve_float_eval(const SkaCurveFloat* curve, f64 t);

#ifdef __cplusplus
}
#endif
