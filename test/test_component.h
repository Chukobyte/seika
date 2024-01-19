#pragma once

typedef struct TestComponent {
    float value;
} TestComponent;

#define SKA_GENERIC_TYPE TestComponent
#include "generic_type_object.h"
#undef SKA_GENERIC_TYPE
