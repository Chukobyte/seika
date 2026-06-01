#if SKA_AUDIO

#include "audio.h"
#include "audio_manager.h"
#include "seika/assert.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include "seika/memory.h"
#include "seika/logger.h"
#include "seika/asset/asset_file_loader.h"

static uint32 audioWavSampleRate = 0;
static bool isAudioInitialized = false;

static bool load_wav_data_from_file(const char* file_path, int32* sample_count, int32* channels, int32* sample_rate, void** samples);

bool ska_audio_initialize() {
    SKA_ASSERT(isAudioInitialized == false);
    ska_audio_manager_init();
    isAudioInitialized = true;
    return true;
}

void ska_audio_finalize() {
    SKA_ASSERT(isAudioInitialized);
    ska_audio_manager_finalize();
    isAudioInitialized = false;
}

void ska_audio_print_audio_source(SkaAudioSource* audioSource) {
    ska_logger_debug("audio source | channels = %d, sample rate = %d, sample count = %d, samples = %x",
                    audioSource->channels, audioSource->sample_rate, audioSource->sample_count, audioSource->samples);
}

void ska_audio_set_wav_sample_rate(uint32 wavSampleRate) {
    audioWavSampleRate = wavSampleRate;
}

uint32 ska_audio_get_wav_sample_rate() {
    return audioWavSampleRate;
}

SkaAudioSource* ska_audio_load_audio_source_wav(const char* fileName) {
    int32 sampleCount;
    int32 channels;
    int32 sampleRate;
    void* samples = NULL;

    if (!load_wav_data_from_file(fileName, &sampleCount, &channels, &sampleRate, &samples)) {
        ska_logger_error("Failed to load audio wav file at '%s'", fileName);
        return NULL;
    }

    SkaAudioSource* newAudioSource = SKA_ALLOC(SkaAudioSource);
    newAudioSource->file_path = fileName;
    newAudioSource->pitch = 1.0;
    newAudioSource->sample_count = sampleCount;
    newAudioSource->channels = channels;
    newAudioSource->sample_rate = sampleRate;
    newAudioSource->samples = samples;

    if ((uint32)newAudioSource->sample_rate != audioWavSampleRate) {
        const int32 inputFrameCount = sampleCount / channels;
        const f64 resampleRatio = (f64)audioWavSampleRate / sampleRate;
        const int32 outputFrameCount = (int32)(inputFrameCount * resampleRatio);

        int16_t* resampledSamples = SKA_ALLOC_BYTES(outputFrameCount * channels * sizeof(int16_t));
        if (!resampledSamples) {
            ska_logger_error("Failed to allocate memory for resampled audio for '%s'!", fileName);
            SKA_FREE(samples);
            SKA_FREE(newAudioSource);
            return NULL;
        }

        ma_data_converter_config converterConfig = ma_data_converter_config_init(
            ma_format_s16, ma_format_s16, channels, channels,
            newAudioSource->sample_rate, audioWavSampleRate
        );
        ma_data_converter converter;
        if (ma_data_converter_init(&converterConfig, NULL, &converter) != MA_SUCCESS) {
            ska_logger_error("Failed to initialize data converter for audio file '%s'!", fileName);
            SKA_FREE(samples);
            SKA_FREE(resampledSamples);
            SKA_FREE(newAudioSource);
            return NULL;
        }

        ma_uint64 inFrames = inputFrameCount;
        ma_uint64 outFrames = outputFrameCount;
        if (ma_data_converter_process_pcm_frames(&converter, resampledSamples, &outFrames, samples, &inFrames) != MA_SUCCESS) {
            ska_logger_error("Data conversion failed for audio file '%s'!", fileName);
            SKA_FREE(samples);
            SKA_FREE(resampledSamples);
            ma_data_converter_uninit(&converter, NULL);
            SKA_FREE(newAudioSource);
            return NULL;
        }
        ma_data_converter_uninit(&converter, NULL);
        ska_logger_debug("Resampled audio for '%s': inFrames = %d, outFrames = %llu", fileName, inputFrameCount, outFrames);
        SKA_FREE(samples);
        newAudioSource->samples = resampledSamples;
        newAudioSource->sample_rate = (int32)audioWavSampleRate;
        newAudioSource->sample_count = (int32)outFrames * channels;
    }
    ska_audio_manager_register_source(newAudioSource);
    return newAudioSource;
}

bool load_wav_data_from_file(const char* file_path, int32* sample_count, int32* channels, int32* sample_rate, void** samples) {
    usize len = 0;
    char* file_data = ska_asset_file_loader_read_file_contents_as_string(file_path, &len);
    ska_logger_debug("file '%s' size '%u' bytes", file_path, len);

    drwav_uint64 totalPcmFrameCount = 0;
    *samples =  drwav_open_memory_and_read_pcm_frames_s16(file_data, len, (uint32*)channels, (uint32*)sample_rate, &totalPcmFrameCount, NULL);
    SKA_FREE(file_data);

    if (!*samples) {
        *samples = NULL;
        ska_logger_error("Could not load .wav file: %s", file_path);
        return false;
    }

    *sample_count = (int32)totalPcmFrameCount * *channels;

    return true;
}

void ska_audio_delete_audio_source_wav(SkaAudioSource* audioSource) {
    ska_audio_manager_unregister_source(audioSource);
    SKA_FREE(audioSource);
}

#endif // #if SKA_AUDIO
