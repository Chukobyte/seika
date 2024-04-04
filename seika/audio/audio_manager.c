#include "audio_manager.h"

#include <stdint.h>
#include <string.h>

#include <miniaudio.h>

#include "audio.h"
#include "seika/memory.h"
#include "seika/logger.h"
#include "seika/assert.h"
#include "seika/asset/asset_manager.h"
#include "seika/thread/pthread.h"

#define SKA_MAX_AUDIO_INSTANCES 32

static void audio_data_callback(ma_device* device, void* output, const void* input, ma_uint32 frame_count);

static ma_device* audio_device = NULL;
static pthread_mutex_t audio_mutex;

// An instance of an RBE audio source
typedef struct SkaAudioInstance {
    SkaAudioSource* source;
    uint32 id;
    bool is_playing;
    bool does_loop;
    f64 sample_position;
} SkaAudioInstance;

typedef struct SkaAudioInstances {
    SkaAudioInstance* instances[SKA_MAX_AUDIO_INSTANCES];
    usize count;
} SkaAudioInstances;

static SkaAudioInstances* audio_instances = NULL;

// --- Audio Manager --- //
bool ska_audio_manager_init(uint32 wavSampleRate) {
    audio_instances = SKA_MEM_ALLOCATE(SkaAudioInstances);
    pthread_mutex_init(&audio_mutex, NULL);
    ska_audio_set_wav_sample_rate(wavSampleRate);
    // Device
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.pDeviceID = NULL;
    config.playback.format = ma_format_s16;
    config.playback.channels = 2;
    config.capture.pDeviceID = NULL;
    config.capture.format = ma_format_s16;
    config.capture.channels = 1;
    config.sampleRate = wavSampleRate;
    config.dataCallback = audio_data_callback;
    config.pUserData = NULL;
    audio_device = SKA_MEM_ALLOCATE(ma_device);
    if (ma_device_init(NULL, &config, audio_device) != MA_SUCCESS) {
        ska_logger_error("Failed to initialize miniaudio device!");
        return false;
    }

    if (ma_device_start(audio_device) != MA_SUCCESS) {
        ska_logger_error("Failed to start audio device!");
        return false;
    }

    return true;
}

void ska_audio_manager_finalize() {
    ma_device_uninit(audio_device);
    SKA_MEM_FREE(audio_device);
    audio_device = NULL;

    SKA_MEM_FREE(audio_instances); // TODO: Properly free up all instances
    audio_instances = NULL;

    pthread_mutex_destroy(&audio_mutex);
}

void ska_audio_manager_play_sound(const char* filePath, bool loops) {
    if (!ska_asset_manager_has_audio_source(filePath)) {
        ska_logger_error("Doesn't have audio source loaded at path '%s' loaded!  Aborting...", filePath);
        return;
    } else if (audio_instances->count >= SKA_MAX_AUDIO_INSTANCES) {
        ska_logger_warn("Reached max audio instances of '%d', not playing sound!", SKA_MAX_AUDIO_INSTANCES);
        return;
    }

    pthread_mutex_lock(&audio_mutex);
    // Create audio instance and add to instances array
    static unsigned int audioInstanceId = 0;  // TODO: temp id for now in case we need to grab a hold of an audio instance for roll back later...
    SkaAudioInstance* audioInstance = SKA_MEM_ALLOCATE(SkaAudioInstance);
    audioInstance->source = ska_asset_manager_get_audio_source(filePath);
    audioInstance->id = audioInstanceId++;
    audioInstance->does_loop = loops;
    audioInstance->sample_position = 0.0f;
    audioInstance->is_playing = true; // Sets sound instance to be played

    audio_instances->instances[audio_instances->count++] = audioInstance;
    ska_logger_debug("Added audio instance from file path '%s' to play!", filePath);
    pthread_mutex_unlock(&audio_mutex);
}

void ska_audio_manager_stop_sound(const char* filePath) {
    pthread_mutex_lock(&audio_mutex);
    for (usize i = 0; i < audio_instances->count; i++) {
        SkaAudioInstance* audioInst = audio_instances->instances[i];
        if (strcmp(audioInst->source->file_path, filePath) == 0) {
            audioInst->is_playing = false;
            break;
        }
    }
    pthread_mutex_unlock(&audio_mutex);
}

// --- Mini Audio Callback --- //
void audio_data_callback(ma_device* device, void* output, const void* input, ma_uint32 frame_count) {
    if (audio_instances->count <= 0) {
        return;
    }

    pthread_mutex_lock(&audio_mutex);
    memset(output, 0, frame_count * device->playback.channels * ma_get_bytes_per_sample(device->playback.format));
    usize removedInstances = 0;
    for (usize i = 0; i < audio_instances->count; i++) {
        SkaAudioInstance* audioInst = audio_instances->instances[i];
        SKA_ASSERT_FMT(audioInst != NULL, "audio instance with index %zu is null!", i);
        if (!audioInst->is_playing) {
            SKA_MEM_FREE(audioInst);
            audio_instances->instances[i] = NULL;
            removedInstances++;
            continue;
        }

        const int32_t channels = audioInst->source->channels;
        const f64 pitch = audioInst->source->pitch;
        int16* sampleOut = (int16*) output;
        int16* samples = (int16*) audioInst->source->samples;
        uint64_t samplesToWrite = (uint64_t) frame_count;

        // Write to output
        for (uint64_t writeSample = 0; writeSample < samplesToWrite; writeSample++) {
            f64 startSamplePosition = audioInst->sample_position;

            f64 targetSamplePosition = startSamplePosition + (f64)channels * pitch;
            if (targetSamplePosition >= audioInst->source->sample_count) {
                targetSamplePosition -= (f64)audioInst->source->sample_count;
            }

            uint64 leftId = (uint64) startSamplePosition;
            if (channels > 1) {
                leftId &= ~((uint64)(0x01));
            }
            const uint64 rightId = leftId + (uint64)(channels - 1);
            const int16 startLeftSample = samples[leftId + channels];
            const int16 startRightSample = samples[rightId + channels];

            const int16 leftSample = (int16)(startLeftSample / channels);
            const int16 rightSample = (int16)(startRightSample / channels);

            *sampleOut++ += leftSample;  // Left
            *sampleOut++ += rightSample; // Right

            // Possibly need fixed sampling instead
            audioInst->sample_position = targetSamplePosition;

            const bool isAtEnd = audioInst->sample_position >= audioInst->source->sample_count - channels - 1;
            if (isAtEnd) {
                audioInst->sample_position = 0;
                if (!audioInst->does_loop) {
                    ska_logger_debug("Audio instance with id '%u' is queued for deletion!", audioInst->id);
                    audio_instances->instances[i] = NULL;
                    removedInstances++;
                    SKA_MEM_FREE(audioInst);
                    break;
                }
            }
        }
    }

    // Reshuffle array and update count if data sources have been removed
    if (removedInstances > 0) {
        static SkaAudioInstance* tempAudioInstances[SKA_MAX_AUDIO_INSTANCES];
        usize newCount = 0;
        // Place non-null instances in temp array
        for (usize i = 0; i < audio_instances->count; i++) {
            if (audio_instances->instances[i] != NULL) {
                tempAudioInstances[newCount++] = audio_instances->instances[i];
            }
        }
        // Now fill up regular array
        for (usize i = 0; i < newCount; i++) {
            audio_instances->instances[i] = tempAudioInstances[i];
        }
        audio_instances->count = newCount;
    }
    pthread_mutex_unlock(&audio_mutex);
}
