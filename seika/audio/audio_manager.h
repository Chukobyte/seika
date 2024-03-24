#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "seika/defines.h"

// --- Audio Manager --- //
bool ska_audio_manager_init(uint32 wavSampleRate);
void ska_audio_manager_finalize();
// Process current jobs for resource manager
void ska_audio_manager_play_sound(const char* filePath, bool loops);
void ska_audio_manager_stop_sound(const char* filePath);

#ifdef __cplusplus
}
#endif
