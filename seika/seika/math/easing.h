#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "seika/defines.h"

typedef enum SkaEaseCurve {
    SKA_EASE_LINEAR,
    SKA_EASE_SINE,
    SKA_EASE_QUAD,
    SKA_EASE_CUBIC,
    SKA_EASE_QUART,
    SKA_EASE_QUINT,
    SKA_EASE_EXPO,
    SKA_EASE_CIRC,
    SKA_EASE_BACK,
    SKA_EASE_ELASTIC,
    SKA_EASE_BOUNCE,
} SkaEaseCurve;

typedef enum SkaEaseMode {
    SKA_EASE_IN,
    SKA_EASE_OUT,
    SKA_EASE_IN_OUT,
} SkaEaseMode;

// Runtime dispatch for data-driven curve selection. 'mode' is ignored for SKA_EASE_LINEAR.
f32 ska_ease_eval(SkaEaseCurve curve, SkaEaseMode mode, f32 t);

f32 ska_ease_linear(f32 t);

f32 ska_ease_sine_in(f32 t);
f32 ska_ease_sine_out(f32 t);
f32 ska_ease_sine_in_out(f32 t);

f32 ska_ease_quad_in(f32 t);
f32 ska_ease_quad_out(f32 t);
f32 ska_ease_quad_in_out(f32 t);

f32 ska_ease_cubic_in(f32 t);
f32 ska_ease_cubic_out(f32 t);
f32 ska_ease_cubic_in_out(f32 t);

f32 ska_ease_quart_in(f32 t);
f32 ska_ease_quart_out(f32 t);
f32 ska_ease_quart_in_out(f32 t);

f32 ska_ease_quint_in(f32 t);
f32 ska_ease_quint_out(f32 t);
f32 ska_ease_quint_in_out(f32 t);

f32 ska_ease_expo_in(f32 t);
f32 ska_ease_expo_out(f32 t);
f32 ska_ease_expo_in_out(f32 t);

f32 ska_ease_circ_in(f32 t);
f32 ska_ease_circ_out(f32 t);
f32 ska_ease_circ_in_out(f32 t);

// Overshoots past 0/1 by design (spring-like pop) -- do not clamp the result.
f32 ska_ease_back_in(f32 t);
f32 ska_ease_back_out(f32 t);
f32 ska_ease_back_in_out(f32 t);

// Overshoots past 0/1 by design (spring-like oscillation) -- do not clamp the result.
f32 ska_ease_elastic_in(f32 t);
f32 ska_ease_elastic_out(f32 t);
f32 ska_ease_elastic_in_out(f32 t);

f32 ska_ease_bounce_in(f32 t);
f32 ska_ease_bounce_out(f32 t);
f32 ska_ease_bounce_in_out(f32 t);

#ifdef __cplusplus
}
#endif
