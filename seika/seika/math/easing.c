#include "easing.h"

#include <math.h>

#include "seika/math/math.h"

// --- Linear --- //
f32 ska_ease_linear(f32 t) {
    return t;
}

// --- Sine --- //
f32 ska_ease_sine_in(f32 t) {
    return 1.0f - cosf(t * SKA_PI * 0.5f);
}

f32 ska_ease_sine_out(f32 t) {
    return sinf(t * SKA_PI * 0.5f);
}

f32 ska_ease_sine_in_out(f32 t) {
    return -(cosf(SKA_PI * t) - 1.0f) * 0.5f;
}

// --- Quad --- //
f32 ska_ease_quad_in(f32 t) {
    return t * t;
}

f32 ska_ease_quad_out(f32 t) {
    return 1.0f - (1.0f - t) * (1.0f - t);
}

f32 ska_ease_quad_in_out(f32 t) {
    if (t < 0.5f) {
        return 2.0f * t * t;
    }
    const f32 f = -2.0f * t + 2.0f;
    return 1.0f - (f * f) * 0.5f;
}

// --- Cubic --- //
f32 ska_ease_cubic_in(f32 t) {
    return t * t * t;
}

f32 ska_ease_cubic_out(f32 t) {
    const f32 f = 1.0f - t;
    return 1.0f - f * f * f;
}

f32 ska_ease_cubic_in_out(f32 t) {
    if (t < 0.5f) {
        return 4.0f * t * t * t;
    }
    const f32 f = -2.0f * t + 2.0f;
    return 1.0f - (f * f * f) * 0.5f;
}

// --- Quart --- //
f32 ska_ease_quart_in(f32 t) {
    return t * t * t * t;
}

f32 ska_ease_quart_out(f32 t) {
    const f32 f = 1.0f - t;
    return 1.0f - f * f * f * f;
}

f32 ska_ease_quart_in_out(f32 t) {
    if (t < 0.5f) {
        return 8.0f * t * t * t * t;
    }
    const f32 f = -2.0f * t + 2.0f;
    return 1.0f - (f * f * f * f) * 0.5f;
}

// --- Quint --- //
f32 ska_ease_quint_in(f32 t) {
    return t * t * t * t * t;
}

f32 ska_ease_quint_out(f32 t) {
    const f32 f = 1.0f - t;
    return 1.0f - f * f * f * f * f;
}

f32 ska_ease_quint_in_out(f32 t) {
    if (t < 0.5f) {
        return 16.0f * t * t * t * t * t;
    }
    const f32 f = -2.0f * t + 2.0f;
    return 1.0f - (f * f * f * f * f) * 0.5f;
}

// --- Expo --- //
f32 ska_ease_expo_in(f32 t) {
    return t <= 0.0f ? 0.0f : powf(2.0f, 10.0f * t - 10.0f);
}

f32 ska_ease_expo_out(f32 t) {
    return t >= 1.0f ? 1.0f : 1.0f - powf(2.0f, -10.0f * t);
}

f32 ska_ease_expo_in_out(f32 t) {
    if (t <= 0.0f) {
        return 0.0f;
    }
    if (t >= 1.0f) {
        return 1.0f;
    }
    if (t < 0.5f) {
        return powf(2.0f, 20.0f * t - 10.0f) * 0.5f;
    }
    return (2.0f - powf(2.0f, -20.0f * t + 10.0f)) * 0.5f;
}

// --- Circ --- //
f32 ska_ease_circ_in(f32 t) {
    return 1.0f - sqrtf(1.0f - t * t);
}

f32 ska_ease_circ_out(f32 t) {
    const f32 f = t - 1.0f;
    return sqrtf(1.0f - f * f);
}

f32 ska_ease_circ_in_out(f32 t) {
    if (t < 0.5f) {
        const f32 f = 2.0f * t;
        return (1.0f - sqrtf(1.0f - f * f)) * 0.5f;
    }
    const f32 f = -2.0f * t + 2.0f;
    return (sqrtf(1.0f - f * f) + 1.0f) * 0.5f;
}

// --- Back --- //
#define SKA_EASE_BACK_C1 1.70158f
#define SKA_EASE_BACK_C3 (SKA_EASE_BACK_C1 + 1.0f)

f32 ska_ease_back_in(f32 t) {
    return SKA_EASE_BACK_C3 * t * t * t - SKA_EASE_BACK_C1 * t * t;
}

f32 ska_ease_back_out(f32 t) {
    const f32 f = t - 1.0f;
    return 1.0f + SKA_EASE_BACK_C3 * f * f * f + SKA_EASE_BACK_C1 * f * f;
}

f32 ska_ease_back_in_out(f32 t) {
    const f32 c2 = SKA_EASE_BACK_C1 * 1.525f;
    if (t < 0.5f) {
        const f32 f = 2.0f * t;
        return (f * f * ((c2 + 1.0f) * f - c2)) * 0.5f;
    }
    const f32 f = 2.0f * t - 2.0f;
    return (f * f * ((c2 + 1.0f) * f + c2) + 2.0f) * 0.5f;
}

// --- Elastic --- //
f32 ska_ease_elastic_in(f32 t) {
    if (t <= 0.0f) {
        return 0.0f;
    }
    if (t >= 1.0f) {
        return 1.0f;
    }
    const f32 c4 = (2.0f * SKA_PI) / 3.0f;
    return -powf(2.0f, 10.0f * t - 10.0f) * sinf((t * 10.0f - 10.75f) * c4);
}

f32 ska_ease_elastic_out(f32 t) {
    if (t <= 0.0f) {
        return 0.0f;
    }
    if (t >= 1.0f) {
        return 1.0f;
    }
    const f32 c4 = (2.0f * SKA_PI) / 3.0f;
    return powf(2.0f, -10.0f * t) * sinf((t * 10.0f - 0.75f) * c4) + 1.0f;
}

f32 ska_ease_elastic_in_out(f32 t) {
    if (t <= 0.0f) {
        return 0.0f;
    }
    if (t >= 1.0f) {
        return 1.0f;
    }
    const f32 c5 = (2.0f * SKA_PI) / 4.5f;
    if (t < 0.5f) {
        return -(powf(2.0f, 20.0f * t - 10.0f) * sinf((20.0f * t - 11.125f) * c5)) * 0.5f;
    }
    return (powf(2.0f, -20.0f * t + 10.0f) * sinf((20.0f * t - 11.125f) * c5)) * 0.5f + 1.0f;
}

// --- Bounce --- //
f32 ska_ease_bounce_out(f32 t) {
    const f32 n1 = 7.5625f;
    const f32 d1 = 2.75f;
    if (t < 1.0f / d1) {
        return n1 * t * t;
    }
    if (t < 2.0f / d1) {
        const f32 f = t - 1.5f / d1;
        return n1 * f * f + 0.75f;
    }
    if (t < 2.5f / d1) {
        const f32 f = t - 2.25f / d1;
        return n1 * f * f + 0.9375f;
    }
    const f32 f = t - 2.625f / d1;
    return n1 * f * f + 0.984375f;
}

f32 ska_ease_bounce_in(f32 t) {
    return 1.0f - ska_ease_bounce_out(1.0f - t);
}

f32 ska_ease_bounce_in_out(f32 t) {
    if (t < 0.5f) {
        return (1.0f - ska_ease_bounce_out(1.0f - 2.0f * t)) * 0.5f;
    }
    return (1.0f + ska_ease_bounce_out(2.0f * t - 1.0f)) * 0.5f;
}

// --- Dispatch --- //
f32 ska_ease_eval(SkaEaseCurve curve, SkaEaseMode mode, f32 t) {
    switch (curve) {
    case SKA_EASE_LINEAR:
        return ska_ease_linear(t);
    case SKA_EASE_SINE:
        switch (mode) {
        case SKA_EASE_IN: return ska_ease_sine_in(t);
        case SKA_EASE_OUT: return ska_ease_sine_out(t);
        case SKA_EASE_IN_OUT: return ska_ease_sine_in_out(t);
        }
        break;
    case SKA_EASE_QUAD:
        switch (mode) {
        case SKA_EASE_IN: return ska_ease_quad_in(t);
        case SKA_EASE_OUT: return ska_ease_quad_out(t);
        case SKA_EASE_IN_OUT: return ska_ease_quad_in_out(t);
        }
        break;
    case SKA_EASE_CUBIC:
        switch (mode) {
        case SKA_EASE_IN: return ska_ease_cubic_in(t);
        case SKA_EASE_OUT: return ska_ease_cubic_out(t);
        case SKA_EASE_IN_OUT: return ska_ease_cubic_in_out(t);
        }
        break;
    case SKA_EASE_QUART:
        switch (mode) {
        case SKA_EASE_IN: return ska_ease_quart_in(t);
        case SKA_EASE_OUT: return ska_ease_quart_out(t);
        case SKA_EASE_IN_OUT: return ska_ease_quart_in_out(t);
        }
        break;
    case SKA_EASE_QUINT:
        switch (mode) {
        case SKA_EASE_IN: return ska_ease_quint_in(t);
        case SKA_EASE_OUT: return ska_ease_quint_out(t);
        case SKA_EASE_IN_OUT: return ska_ease_quint_in_out(t);
        }
        break;
    case SKA_EASE_EXPO:
        switch (mode) {
        case SKA_EASE_IN: return ska_ease_expo_in(t);
        case SKA_EASE_OUT: return ska_ease_expo_out(t);
        case SKA_EASE_IN_OUT: return ska_ease_expo_in_out(t);
        }
        break;
    case SKA_EASE_CIRC:
        switch (mode) {
        case SKA_EASE_IN: return ska_ease_circ_in(t);
        case SKA_EASE_OUT: return ska_ease_circ_out(t);
        case SKA_EASE_IN_OUT: return ska_ease_circ_in_out(t);
        }
        break;
    case SKA_EASE_BACK:
        switch (mode) {
        case SKA_EASE_IN: return ska_ease_back_in(t);
        case SKA_EASE_OUT: return ska_ease_back_out(t);
        case SKA_EASE_IN_OUT: return ska_ease_back_in_out(t);
        }
        break;
    case SKA_EASE_ELASTIC:
        switch (mode) {
        case SKA_EASE_IN: return ska_ease_elastic_in(t);
        case SKA_EASE_OUT: return ska_ease_elastic_out(t);
        case SKA_EASE_IN_OUT: return ska_ease_elastic_in_out(t);
        }
        break;
    case SKA_EASE_BOUNCE:
        switch (mode) {
        case SKA_EASE_IN: return ska_ease_bounce_in(t);
        case SKA_EASE_OUT: return ska_ease_bounce_out(t);
        case SKA_EASE_IN_OUT: return ska_ease_bounce_in_out(t);
        }
        break;
    }
    return t;
}
