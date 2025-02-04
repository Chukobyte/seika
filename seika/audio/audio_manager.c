#if SKA_AUDIO

#include "audio_manager.h"

#include <string.h>
#include <math.h>

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
bool ska_audio_manager_init() {
    audio_instances = SKA_ALLOC_ZEROED(SkaAudioInstances);
    pthread_mutex_init(&audio_mutex, NULL);
    // Device
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.pDeviceID = NULL;
    config.playback.format = ma_format_s16;
    config.playback.channels = 2;
    config.capture.pDeviceID = NULL;
    config.capture.format = ma_format_s16;
    config.capture.channels = 1;
    config.sampleRate = 0; // Allow miniaudio to choose sample rate
    config.dataCallback = audio_data_callback;
    config.pUserData = NULL;
    audio_device = SKA_ALLOC(ma_device);
    if (ma_device_init(NULL, &config, audio_device) != MA_SUCCESS) {
        ska_logger_error("Failed to initialize miniaudio device!");
        return false;
    }

    if (ma_device_start(audio_device) != MA_SUCCESS) {
        ska_logger_error("Failed to start audio device!");
        return false;
    }

    ska_audio_set_wav_sample_rate(audio_device->sampleRate);

    return true;
}

void ska_audio_manager_finalize() {
    ma_device_uninit(audio_device);
    SKA_FREE(audio_device);
    audio_device = NULL;

    SKA_FREE(audio_instances); // TODO: Properly free up all instances
    audio_instances = NULL;

    pthread_mutex_destroy(&audio_mutex);
}

void ska_audio_manager_play_sound(SkaAudioSource* audioSource, bool loops) {
    if (audio_instances->count >= SKA_MAX_AUDIO_INSTANCES) {
        ska_logger_warn("Reached max audio instances of '%d', not playing sound!", SKA_MAX_AUDIO_INSTANCES);
        return;
    }
    pthread_mutex_lock(&audio_mutex);
    // Create audio instance and add to instances array
    static uint32 audioInstanceId = 0;  // TODO: temp id for now in case we need to grab a hold of an audio instance for roll back later...
    SkaAudioInstance* audioInstance = SKA_ALLOC(SkaAudioInstance);
    audioInstance->source = audioSource;
    audioInstance->id = audioInstanceId++;
    audioInstance->does_loop = loops;
    audioInstance->sample_position = 0.0f;
    audioInstance->is_playing = true; // Sets sound instance to be played

    audio_instances->instances[audio_instances->count++] = audioInstance;
    ska_logger_debug("Added audio instance from file path '%s' to play!", audioSource->file_path);
    pthread_mutex_unlock(&audio_mutex);
}

void ska_audio_manager_play_sound2(const char* filePath, bool loops) {
    if (!ska_asset_manager_has_audio_source(filePath)) {
        ska_logger_error("Doesn't have audio source loaded at path '%s' loaded!  Aborting...", filePath);
        return;
    }
    SkaAudioSource* audioSource = ska_asset_manager_get_audio_source(filePath);
    ska_audio_manager_play_sound(audioSource, loops);
}

void ska_audio_manager_stop_sound(SkaAudioSource* audioSource) {
    pthread_mutex_lock(&audio_mutex);
    for (usize i = 0; i < audio_instances->count; i++) {
        SkaAudioInstance* audioInst = audio_instances->instances[i];
        if (audioInst->source == audioSource) {
            audioInst->is_playing = false;
            break;
        }
    }
    pthread_mutex_unlock(&audio_mutex);
}

void ska_audio_manager_stop_sound2(const char* filePath) {
    SkaAudioSource* audioSource = ska_asset_manager_get_audio_source(filePath);
    ska_audio_manager_stop_sound(audioSource);
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
            SKA_FREE(audioInst);
            audio_instances->instances[i] = NULL;
            removedInstances++;
            continue;
        }

        const int32 channels = audioInst->source->channels;
        const f64 pitch = audioInst->source->pitch;
        int16* sampleOut = (int16*) output;
        int16* samples = (int16*) audioInst->source->samples;
        uint64 samplesToWrite = (uint64) frame_count;

        // Write to output
        for (uint64 writeSample = 0; writeSample < samplesToWrite; writeSample++) {
            const f64 startSamplePosition = audioInst->sample_position;

            uint64 curIndex = (uint64) startSamplePosition;
            if (channels > 1) {
                curIndex &= ~((uint64)0x01);
            }
            const uint64 nextIndex = (curIndex + channels) % audioInst->source->sample_count;
            const uint64 leftIndexNext = nextIndex;
            const uint64 rightIndexNext = nextIndex + 1;
            const int16 sampleLeftNext = samples[leftIndexNext];
            const int16 sampleRightNext = samples[rightIndexNext];

            *sampleOut++ += sampleLeftNext;
            *sampleOut++ += sampleRightNext;

            // Update the instance's sample position and clamp it
            const f64 targetSamplePosition = startSamplePosition + (f64)channels * pitch;
            audioInst->sample_position = fmod(targetSamplePosition, audioInst->source->sample_count);

            // Check if the raw target sample position has passed boundary
            if (targetSamplePosition >= (f64)audioInst->source->sample_count - channels) {
                audioInst->sample_position = 0;
                if (!audioInst->does_loop) {
                    ska_logger_debug("Audio instance with id '%u' is queued for deletion!", audioInst->id);
                    audio_instances->instances[i] = NULL;
                    removedInstances++;
                    SKA_FREE(audioInst);
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

#endif // #if SKA_AUDIO
