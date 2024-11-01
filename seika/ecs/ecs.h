#pragma once

#if SKA_ECS

// Including ecs related headers to simplify includes
#include "ec_system.h"

void ska_ecs_initialize();
void ska_ecs_finalize();

#endif // if SKA_ECS