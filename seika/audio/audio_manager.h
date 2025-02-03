#pragma once

#if SKA_AUDIO

#ifdef __cplusplus
extern "C" {
#endif

#include "seika/defines.h"

struct SkaAudioSource;

// --- Audio Manager --- //
bool ska_audio_manager_init();
void ska_audio_manager_finalize();
// Process current jobs for resource manager
void ska_audio_manager_play_sound(struct SkaAudioSource* audioSource, bool loops);
void ska_audio_manager_play_sound2(const char* filePath, bool loops);
void ska_audio_manager_stop_sound(struct SkaAudioSource* audioSource);
void ska_audio_manager_stop_sound2(const char* filePath);

#ifdef __cplusplus
}
#endif

#endif // #if SKA_AUDIO
