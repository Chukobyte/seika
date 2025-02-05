#pragma once

#include "seika/macro_utils.h"

#define SKA_VERSION_MAJOR 0
#define SKA_VERSION_MINOR 2
#define SKA_VERSION_PATCH 16

#define SKA_VERSION (SKA_MACRO_TO_STRING(SKA_VERSION_MAJOR) "." SKA_MACRO_TO_STRING(SKA_VERSION_MINOR) "." SKA_MACRO_TO_STRING(SKA_VERSION_PATCH))
