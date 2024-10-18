#include "audio.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include "seika/memory.h"
#include "seika/logger.h"
#include "seika/asset/asset_file_loader.h"

static uint32 audioWavSampleRate = SKA_AUDIO_SOURCE_DEFAULT_WAV_SAMPLE_RATE;

static bool load_wav_data_from_file(const char* file_path, int32* sample_count, int32* channels, int32* sample_rate, void** samples);

void ska_audio_print_audio_source(SkaAudioSource* audioSource) {
    ska_logger_debug("audio source | channels = %d, sample rate = %d, sample count = %d, samples = %x",
                    audioSource->channels, audioSource->sample_rate, audioSource->sample_count, audioSource->samples);
}

void ska_audio_set_wav_sample_rate(uint32 wavSampleRate) {
    audioWavSampleRate = wavSampleRate;
}

uint32_t ska_audio_get_wav_sample_rate() {
    return audioWavSampleRate;
}

SkaAudioSource* ska_audio_load_audio_source_wav(const char* fileName) {
    int32_t sampleCount;
    int32_t channels;
    int32_t sampleRate;
    void* samples = NULL;
    if (!load_wav_data_from_file(fileName, &sampleCount, &channels, &sampleRate, &samples)) {
        ska_logger_error("Failed to load audio wav file at '%s'", fileName);
        return NULL;
    }
    SkaAudioSource* newAudioSource = (SkaAudioSource*)SKA_MEM_ALLOCATE_SIZE(sizeof(SkaAudioSource*) + (sampleCount * sizeof(int16_t*)));
    newAudioSource->file_path = fileName;
    newAudioSource->pitch = 1.0;
    newAudioSource->sample_count = sampleCount;
    newAudioSource->channels = channels;
    newAudioSource->sample_rate = sampleRate;
    newAudioSource->samples = samples;

    if ((uint32_t)newAudioSource->sample_rate != audioWavSampleRate) {
        ska_logger_error("Sample rate for wav file '%s' is %d instead of the expected sample rate of %d!  Audio won't play as expected!",
                        fileName, sampleRate, audioWavSampleRate);
    }

    return newAudioSource;
}

bool load_wav_data_from_file(const char* file_path, int32_t* sample_count, int32_t* channels, int32_t* sample_rate, void** samples) {
    usize len = 0;
    char* file_data = ska_asset_file_loader_read_file_contents_as_string(file_path, &len);
    ska_logger_debug("file '%s' size '%u' bytes", file_path, len);

    drwav_uint64 totalPcmFrameCount = 0;
    *samples =  drwav_open_memory_and_read_pcm_frames_s16(file_data, len, (uint32_t*)channels, (uint32_t*)sample_rate, &totalPcmFrameCount, NULL);
    SKA_MEM_FREE(file_data);

    if (!*samples) {
        *samples = NULL;
        ska_logger_error("Could not load .wav file: %s", file_path);
        return false;
    }

    *sample_count = (int32_t) totalPcmFrameCount * *channels;

    return true;
}
