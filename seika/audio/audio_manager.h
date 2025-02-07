#pragma once

// TODO: Combine this with audio.h

#if SKA_AUDIO

#ifdef __cplusplus
extern "C" {
#endif

#include "seika/defines.h"

struct SkaAudioSource;

// --- Audio Manager --- //
bool ska_audio_manager_init();
void ska_audio_manager_finalize();
void ska_audio_manager_register_source(struct SkaAudioSource *source);
void ska_audio_manager_unregister_source(struct SkaAudioSource *source);
// Audio functions

void ska_audio_manager_play_sound(struct SkaAudioSource* audioSource, bool loops);
void ska_audio_manager_play_sound2(const char* filePath, bool loops);
void ska_audio_manager_stop_sound(struct SkaAudioSource* audioSource);
void ska_audio_manager_stop_sound2(const char* filePath);
f32 ska_audio_manager_get_position(struct SkaAudioSource* audioSource);
f32 ska_audio_manager_get_position_seconds(struct SkaAudioSource* audioSource);
#ifdef __cplusplus
}
#endif

#endif // #if SKA_AUDIO
