#pragma once

#if SKA_AUDIO

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "seika/defines.h"

typedef struct SkaAudioSource {
    const char* file_path;
    f64 pitch;
    int32 channels;
    int32 sample_rate;
    void* samples;
    int32 sample_count;
    uint32 dataId;
} SkaAudioSource;

bool ska_audio_initialize();
void ska_audio_finalize();
void ska_audio_print_audio_source(SkaAudioSource* audioSource);
void ska_audio_set_wav_sample_rate(uint32 wavSampleRate);
uint32 ska_audio_get_wav_sample_rate();
SkaAudioSource* ska_audio_load_audio_source_wav(const char* fileName);
void ska_audio_delete_audio_source_wav(SkaAudioSource* audioSource);

#ifdef __cplusplus
}
#endif

#endif // #if SKA_AUDIO
