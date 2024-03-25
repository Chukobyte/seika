#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "seika/defines.h"

#define SKA_AUDIO_SOURCE_DEFAULT_WAV_SAMPLE_RATE 44100

typedef struct SkaAudioSource {
    const char* file_path;
    f64 pitch;
    int32 channels;
    int32 sample_rate;
    void* samples;
    int32 sample_count;
} SkaAudioSource;

void ska_audio_print_audio_source(SkaAudioSource* audioSource);
void ska_audio_set_wav_sample_rate(uint32 wavSampleRate);
uint32 ska_audio_get_wav_sample_rate();
SkaAudioSource* ska_audio_load_audio_source_wav(const char* fileName);

#ifdef __cplusplus
}
#endif
