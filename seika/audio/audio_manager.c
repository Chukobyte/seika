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

#define SKA_MAX_AUDIO_SOURCES 64
#define SKA_MAX_AUDIO_INSTANCES 32
// Upper bound on how many frames are mixed at once; the device callback's frame_count is
// processed in chunks of this size so the float scratch/mix buffers can stay static (no
// allocation on the real-time audio thread) regardless of the device's chosen period size.
#define SKA_AUDIO_MAX_FRAME_CHUNK 4096

static void audio_data_callback(ma_device* device, void* output, const void* input, ma_uint32 frame_count);

static ma_device* audio_device = NULL;
static ma_context* null_audio_context = NULL; // only set when output goes to the null device
static bool use_null_device = false;
static pthread_mutex_t audio_mutex;
static ma_spatializer_listener audio_listener;
static f32 master_volume = 1.0f;

// An instance of a Ska Audio Source
typedef struct SkaAudioInstance {
    SkaAudioSource* source;
    SkaAudioInstanceId id;
    bool is_playing;
    bool is_paused;
    bool does_loop;
    f64 sample_position;
    f32 volume;
    f32 pitch;
    f32 pan;
    bool is_3d;
    ma_spatializer spatializer; // only initialized/valid when is_3d is true
} SkaAudioInstance;

typedef struct SkaAudioInstances {
    SkaAudioInstance* instances[SKA_MAX_AUDIO_INSTANCES];
    usize count;
} SkaAudioInstances;

// Indexed by SkaAudioSource.data_id. Holds no source pointer: the asset manager stores sources
// by value, so the struct passed to register isn't the one later calls see.
typedef struct SkaAudioSourceData {
    SkaAudioInstance* last_spawned_instance;
} SkaAudioSourceData;

static SkaAudioInstances* audio_instances = NULL;
static SkaAudioSourceData audio_source_data[SKA_MAX_AUDIO_SOURCES] = {0};
static usize audio_source_count = 0;
static SkaAudioInstanceId next_audio_instance_id = 0;

// --- Audio Manager --- //

static inline SkaAudioSourceData* getSourceData(SkaAudioSource* source) {
    return &audio_source_data[source->data_id];
}

static ma_attenuation_model to_ma_attenuation_model(SkaAudioAttenuationModel model) {
    switch (model) {
        case SkaAudioAttenuationModel_NONE: return ma_attenuation_model_none;
        case SkaAudioAttenuationModel_LINEAR: return ma_attenuation_model_linear;
        case SkaAudioAttenuationModel_EXPONENTIAL: return ma_attenuation_model_exponential;
        case SkaAudioAttenuationModel_INVERSE:
        default: return ma_attenuation_model_inverse;
    }
}

static inline f32 sanitize_volume(f32 volume) {
    return volume > 0.0f ? volume : 0.0f; // also maps NaN to 0
}

static inline f32 sanitize_pitch(f32 pitch) {
    return pitch > 0.0f ? pitch : 1.0f; // 0/negative/NaN would stall or run sample_position backwards
}

static inline f32 clamp_pan(f32 pan) {
    if (!(pan > -1.0f)) {
        return pan < 0.0f ? -1.0f : 0.0f; // NaN -> center
    }
    return pan > 1.0f ? 1.0f : pan;
}

// Caller must hold audio_mutex. Does not add the 3D spatializer; the caller sets is_3d/spatializer.
static SkaAudioInstance* create_audio_instance_locked(SkaAudioSource* audio_source, SkaAudioPlayParams play_params) {
    SkaAudioInstance* audio_instance = SKA_ALLOC(SkaAudioInstance);
    audio_instance->source = audio_source;
    audio_instance->id = ++next_audio_instance_id; // ids start at 1; 0 is SKA_AUDIO_INVALID_INSTANCE_ID
    audio_instance->does_loop = play_params.loops;
    audio_instance->sample_position = 0.0;
    audio_instance->is_playing = true;
    audio_instance->is_paused = false;
    audio_instance->volume = sanitize_volume(play_params.volume);
    audio_instance->pitch = sanitize_pitch(play_params.pitch);
    audio_instance->pan = 0.0f;
    audio_instance->is_3d = false;
    audio_instances->instances[audio_instances->count++] = audio_instance;
    return audio_instance;
}

// Caller must hold audio_mutex.
static SkaAudioInstance* find_instance_by_id_locked(SkaAudioInstanceId instance_id) {
    if (instance_id == SKA_AUDIO_INVALID_INSTANCE_ID) {
        return NULL;
    }
    for (usize i = 0; i < audio_instances->count; i++) {
        SkaAudioInstance* instance = audio_instances->instances[i];
        if (instance != NULL && instance->id == instance_id) {
            return instance;
        }
    }
    return NULL;
}

bool ska_audio_manager_init() {
    audio_instances = SKA_ALLOC_ZEROED(SkaAudioInstances);
    pthread_mutex_init(&audio_mutex, NULL);

    ma_spatializer_listener_config listener_config = ma_spatializer_listener_config_init(2);
    if (ma_spatializer_listener_init(&listener_config, NULL, &audio_listener) != MA_SUCCESS) {
        ska_logger_error("Failed to initialize audio spatializer listener!");
        return false;
    }
    ma_spatializer_listener_set_position(&audio_listener, 0.0f, 0.0f, 0.0f);
    ma_spatializer_listener_set_direction(&audio_listener, 0.0f, 0.0f, -1.0f);
    ma_spatializer_listener_set_world_up(&audio_listener, 0.0f, 1.0f, 0.0f);
    ma_spatializer_listener_set_velocity(&audio_listener, 0.0f, 0.0f, 0.0f);

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
    bool device_ready = !use_null_device && ma_device_init(NULL, &config, audio_device) == MA_SUCCESS;
    if (!device_ready) {
        if (!use_null_device) {
            ska_logger_warn("No audio playback device available, audio output will be silent!");
        }
        // Keeps the whole mixer/instance pipeline alive (callbacks still run) with nothing audible
        const ma_backend null_backend = ma_backend_null;
        null_audio_context = SKA_ALLOC(ma_context);
        if (ma_context_init(&null_backend, 1, NULL, null_audio_context) != MA_SUCCESS) {
            ska_logger_error("Failed to initialize null audio context!");
            SKA_FREE(null_audio_context);
            null_audio_context = NULL;
            SKA_FREE(audio_device);
            audio_device = NULL;
            return false;
        }
        if (ma_device_init(null_audio_context, &config, audio_device) != MA_SUCCESS) {
            ska_logger_error("Failed to initialize null audio device!");
            ma_context_uninit(null_audio_context);
            SKA_FREE(null_audio_context);
            null_audio_context = NULL;
            SKA_FREE(audio_device);
            audio_device = NULL;
            return false;
        }
    }

    if (ma_device_start(audio_device) != MA_SUCCESS) {
        ska_logger_error("Failed to start audio device!");
        return false;
    }

    ska_audio_set_wav_sample_rate(audio_device->sampleRate);

    return true;
}

static inline void free_audio_instance(SkaAudioInstance* audio_inst);

void ska_audio_manager_finalize() {
    if (audio_device) {
        ma_device_uninit(audio_device);
        SKA_FREE(audio_device);
        audio_device = NULL;
    }
    if (null_audio_context) {
        ma_context_uninit(null_audio_context);
        SKA_FREE(null_audio_context);
        null_audio_context = NULL;
    }

    ma_spatializer_listener_uninit(&audio_listener, NULL);

    // Device is stopped, so the callback can no longer touch these
    for (usize i = 0; i < audio_instances->count; i++) {
        free_audio_instance(audio_instances->instances[i]);
    }
    SKA_FREE(audio_instances);
    audio_instances = NULL;
    // Sources are re-registered by the next init (e.g. the next embedding VM), so start ids over
    memset(audio_source_data, 0, sizeof(audio_source_data));
    audio_source_count = 0;
    master_volume = 1.0f;

    pthread_mutex_destroy(&audio_mutex);
}

void ska_audio_manager_set_use_null_device(bool use_null) {
    use_null_device = use_null;
}

void ska_audio_manager_register_source(SkaAudioSource* source) {
    SKA_ASSERT(audio_source_count < SKA_MAX_AUDIO_SOURCES);
    source->data_id = (uint32)audio_source_count++;
    SkaAudioSourceData* data = getSourceData(source);
    data->last_spawned_instance = NULL;
}

void ska_audio_manager_unregister_source(SkaAudioSource* source) {
    // TODO: Implement
    SkaAudioSourceData* data = getSourceData(source);
    data->last_spawned_instance = NULL;
    if (audio_source_count > 0) {
        audio_source_count--;
    }
}

SkaAudioInstanceId ska_audio_manager_play_sound(SkaAudioSource* audio_source, bool loops) {
    SkaAudioPlayParams play_params = SKA_AUDIO_PLAY_PARAMS_DEFAULT;
    play_params.loops = loops;
    return ska_audio_manager_play_sound_with_params(audio_source, play_params);
}

SkaAudioInstanceId ska_audio_manager_play_sound_with_params(SkaAudioSource* audio_source, SkaAudioPlayParams play_params) {
    pthread_mutex_lock(&audio_mutex);
    if (audio_instances->count >= SKA_MAX_AUDIO_INSTANCES) {
        pthread_mutex_unlock(&audio_mutex);
        ska_logger_warn("Reached max audio instances of '%d', not playing sound!", SKA_MAX_AUDIO_INSTANCES);
        return SKA_AUDIO_INVALID_INSTANCE_ID;
    }
    SkaAudioInstance* audio_instance = create_audio_instance_locked(audio_source, play_params);

    SkaAudioSourceData* data = getSourceData(audio_source);
    data->last_spawned_instance = audio_instance;

    ska_logger_debug("Added audio instance from file path '%s' to play!", audio_source->file_path);
    const SkaAudioInstanceId id = audio_instance->id;
    pthread_mutex_unlock(&audio_mutex);
    return id;
}

SkaAudioInstanceId ska_audio_manager_play_sound2(const char* file_path, bool loops) {
    if (!ska_asset_manager_has_audio_source(file_path)) {
        ska_logger_error("Doesn't have audio source loaded at path '%s' loaded!  Aborting...", file_path);
        return SKA_AUDIO_INVALID_INSTANCE_ID;
    }
    SkaAudioSource* audio_source = ska_asset_manager_get_audio_source(file_path);
    return ska_audio_manager_play_sound(audio_source, loops);
}

void ska_audio_manager_stop_sound(SkaAudioSource* audio_source) {
    pthread_mutex_lock(&audio_mutex);
    for (usize i = 0; i < audio_instances->count; i++) {
        SkaAudioInstance* audio_inst = audio_instances->instances[i];
        if (audio_inst->source == audio_source) {
            audio_inst->is_playing = false;
        }
    }
    pthread_mutex_unlock(&audio_mutex);
}

void ska_audio_manager_stop_sound2(const char* file_path) {
    SkaAudioSource* audio_source = ska_asset_manager_get_audio_source(file_path);
    ska_audio_manager_stop_sound(audio_source);
}

f32 ska_audio_manager_get_position(SkaAudioSource* audio_source) {
    f32 position = 0.0f;
    pthread_mutex_lock(&audio_mutex);
    const SkaAudioSourceData* data = getSourceData(audio_source);
    if (data->last_spawned_instance && data->last_spawned_instance->sample_position > 0.0) {
        position = (f32)(data->last_spawned_instance->sample_position / audio_source->sample_count);
    }
    pthread_mutex_unlock(&audio_mutex);
    return position;
}

f32 ska_audio_manager_get_position_seconds(SkaAudioSource* audio_source) {
    f32 seconds = 0.0f;
    pthread_mutex_lock(&audio_mutex);
    const SkaAudioSourceData* data = getSourceData(audio_source);
    if (data->last_spawned_instance && data->last_spawned_instance->sample_position > 0.0) {
        const int32 channels = audio_source->channels;
        const uint32 global_sample_rate = ska_audio_get_wav_sample_rate();
        seconds = (f32)data->last_spawned_instance->sample_position / (f32)(channels * global_sample_rate);
    }
    pthread_mutex_unlock(&audio_mutex);
    return seconds;
}

void ska_audio_manager_set_master_volume(f32 volume) {
    pthread_mutex_lock(&audio_mutex);
    master_volume = sanitize_volume(volume);
    pthread_mutex_unlock(&audio_mutex);
}

f32 ska_audio_manager_get_master_volume() {
    pthread_mutex_lock(&audio_mutex);
    const f32 volume = master_volume;
    pthread_mutex_unlock(&audio_mutex);
    return volume;
}

// --- Spatial (3D/2D positional) Audio --- //

void ska_audio_manager_set_listener(SkaVector3 position, SkaVector3 forward, SkaVector3 up, SkaVector3 velocity) {
    pthread_mutex_lock(&audio_mutex);
    ma_spatializer_listener_set_position(&audio_listener, position.x, position.y, position.z);
    ma_spatializer_listener_set_direction(&audio_listener, forward.x, forward.y, forward.z);
    ma_spatializer_listener_set_world_up(&audio_listener, up.x, up.y, up.z);
    ma_spatializer_listener_set_velocity(&audio_listener, velocity.x, velocity.y, velocity.z);
    pthread_mutex_unlock(&audio_mutex);
}

SkaAudioInstanceId ska_audio_manager_play_sound_3d(SkaAudioSource* audio_source, bool loops, SkaVector3 position, SkaAudio3DParams params) {
    SkaAudioPlayParams play_params = SKA_AUDIO_PLAY_PARAMS_DEFAULT;
    play_params.loops = loops;
    return ska_audio_manager_play_sound_3d_with_params(audio_source, play_params, position, params);
}

SkaAudioInstanceId ska_audio_manager_play_sound_3d_with_params(SkaAudioSource* audio_source, SkaAudioPlayParams play_params, SkaVector3 position, SkaAudio3DParams params) {
    pthread_mutex_lock(&audio_mutex);
    if (audio_instances->count >= SKA_MAX_AUDIO_INSTANCES) {
        pthread_mutex_unlock(&audio_mutex);
        ska_logger_warn("Reached max audio instances of '%d', not playing sound!", SKA_MAX_AUDIO_INSTANCES);
        return SKA_AUDIO_INVALID_INSTANCE_ID;
    }
    SkaAudioInstance* audio_instance = create_audio_instance_locked(audio_source, play_params);

    ma_spatializer_config spatializer_config = ma_spatializer_config_init(1, 2); // mono in, stereo out
    spatializer_config.attenuationModel = to_ma_attenuation_model(params.attenuation_model);
    spatializer_config.positioning = ma_positioning_absolute;
    spatializer_config.minDistance = params.min_distance;
    spatializer_config.maxDistance = params.max_distance;
    spatializer_config.rolloff = params.rolloff;
    spatializer_config.dopplerFactor = params.doppler_factor;

    if (ma_spatializer_init(&spatializer_config, NULL, &audio_instance->spatializer) != MA_SUCCESS) {
        ska_logger_error("Failed to initialize spatializer for audio instance, playing non-spatialized!");
        audio_instances->count--;
        audio_instances->instances[audio_instances->count] = NULL;
        SKA_FREE(audio_instance);
        pthread_mutex_unlock(&audio_mutex);
        return SKA_AUDIO_INVALID_INSTANCE_ID;
    }
    audio_instance->is_3d = true;
    ma_spatializer_set_position(&audio_instance->spatializer, position.x, position.y, position.z);

    SkaAudioSourceData* data = getSourceData(audio_source);
    data->last_spawned_instance = audio_instance;

    ska_logger_debug("Added 3D audio instance from file path '%s' to play!", audio_source->file_path);
    const SkaAudioInstanceId id = audio_instance->id;
    pthread_mutex_unlock(&audio_mutex);
    return id;
}

void ska_audio_manager_set_instance_position(SkaAudioInstanceId instance_id, SkaVector3 position) {
    pthread_mutex_lock(&audio_mutex);
    SkaAudioInstance* instance = find_instance_by_id_locked(instance_id);
    if (instance != NULL && instance->is_3d) {
        ma_spatializer_set_position(&instance->spatializer, position.x, position.y, position.z);
    }
    pthread_mutex_unlock(&audio_mutex);
}

void ska_audio_manager_set_instance_velocity(SkaAudioInstanceId instance_id, SkaVector3 velocity) {
    pthread_mutex_lock(&audio_mutex);
    SkaAudioInstance* instance = find_instance_by_id_locked(instance_id);
    if (instance != NULL && instance->is_3d) {
        ma_spatializer_set_velocity(&instance->spatializer, velocity.x, velocity.y, velocity.z);
    }
    pthread_mutex_unlock(&audio_mutex);
}

void ska_audio_manager_stop_instance(SkaAudioInstanceId instance_id) {
    pthread_mutex_lock(&audio_mutex);
    SkaAudioInstance* instance = find_instance_by_id_locked(instance_id);
    if (instance != NULL) {
        instance->is_playing = false;
    }
    pthread_mutex_unlock(&audio_mutex);
}

bool ska_audio_manager_is_instance_playing(SkaAudioInstanceId instance_id) {
    pthread_mutex_lock(&audio_mutex);
    SkaAudioInstance* instance = find_instance_by_id_locked(instance_id);
    const bool result = instance != NULL && instance->is_playing;
    pthread_mutex_unlock(&audio_mutex);
    return result;
}

void ska_audio_manager_set_instance_paused(SkaAudioInstanceId instance_id, bool paused) {
    pthread_mutex_lock(&audio_mutex);
    SkaAudioInstance* instance = find_instance_by_id_locked(instance_id);
    if (instance != NULL) {
        instance->is_paused = paused;
    }
    pthread_mutex_unlock(&audio_mutex);
}

bool ska_audio_manager_is_instance_paused(SkaAudioInstanceId instance_id) {
    pthread_mutex_lock(&audio_mutex);
    SkaAudioInstance* instance = find_instance_by_id_locked(instance_id);
    const bool result = instance != NULL && instance->is_playing && instance->is_paused;
    pthread_mutex_unlock(&audio_mutex);
    return result;
}

void ska_audio_manager_set_instance_volume(SkaAudioInstanceId instance_id, f32 volume) {
    pthread_mutex_lock(&audio_mutex);
    SkaAudioInstance* instance = find_instance_by_id_locked(instance_id);
    if (instance != NULL) {
        instance->volume = sanitize_volume(volume);
    }
    pthread_mutex_unlock(&audio_mutex);
}

f32 ska_audio_manager_get_instance_volume(SkaAudioInstanceId instance_id) {
    pthread_mutex_lock(&audio_mutex);
    SkaAudioInstance* instance = find_instance_by_id_locked(instance_id);
    const f32 result = instance != NULL && instance->is_playing ? instance->volume : 0.0f;
    pthread_mutex_unlock(&audio_mutex);
    return result;
}

void ska_audio_manager_set_instance_pitch(SkaAudioInstanceId instance_id, f32 pitch) {
    pthread_mutex_lock(&audio_mutex);
    SkaAudioInstance* instance = find_instance_by_id_locked(instance_id);
    if (instance != NULL) {
        instance->pitch = sanitize_pitch(pitch);
    }
    pthread_mutex_unlock(&audio_mutex);
}

f32 ska_audio_manager_get_instance_pitch(SkaAudioInstanceId instance_id) {
    pthread_mutex_lock(&audio_mutex);
    SkaAudioInstance* instance = find_instance_by_id_locked(instance_id);
    const f32 result = instance != NULL && instance->is_playing ? instance->pitch : 0.0f;
    pthread_mutex_unlock(&audio_mutex);
    return result;
}

void ska_audio_manager_set_instance_pan(SkaAudioInstanceId instance_id, f32 pan) {
    pthread_mutex_lock(&audio_mutex);
    SkaAudioInstance* instance = find_instance_by_id_locked(instance_id);
    if (instance != NULL && !instance->is_3d) {
        instance->pan = clamp_pan(pan);
    }
    pthread_mutex_unlock(&audio_mutex);
}

f32 ska_audio_manager_get_instance_pan(SkaAudioInstanceId instance_id) {
    pthread_mutex_lock(&audio_mutex);
    SkaAudioInstance* instance = find_instance_by_id_locked(instance_id);
    const f32 result = instance != NULL && instance->is_playing ? instance->pan : 0.0f;
    pthread_mutex_unlock(&audio_mutex);
    return result;
}

// --- Mini Audio Callback --- //

// Fills 'out_mono' with 'frame_count' pitch-resampled mono samples in [-1, 1] for a 3D
// instance (downmixing stereo sources), advancing/wrapping sample_position exactly like the
// non-spatialized path below. Zero-fills any trailing frames once a non-looping clip ends.
static void fetch_mono_pitched_samples(SkaAudioInstance* audio_inst, f32* out_mono, ma_uint32 frame_count, bool* out_finished) {
    const int32 channels = audio_inst->source->channels;
    const int32 sample_count = audio_inst->source->sample_count;
    // One callback of doppler-pitch latency: dopplerPitch is recomputed by
    // ma_spatializer_process_pcm_frames() below from this callback's listener/emitter state.
    const f64 pitch = audio_inst->source->pitch * (f64)audio_inst->pitch * (f64)audio_inst->spatializer.dopplerPitch;
    const int16* samples = (const int16*)audio_inst->source->samples;
    *out_finished = false;

    ma_uint32 f = 0;
    for (; f < frame_count; f++) {
        const f64 start_sample_position = audio_inst->sample_position;
        uint64 idx = (uint64)start_sample_position;
        if (channels > 1) {
            idx &= ~((uint64)0x01);
        }
        const int16 left = samples[idx];
        f32 mono_sample;
        if (channels > 1) {
            const int16 right = samples[idx + 1];
            mono_sample = ((f32)left + (f32)right) * 0.5f;
        } else {
            mono_sample = (f32)left;
        }
        out_mono[f] = mono_sample / 32768.0f;

        const f64 target_sample_position = start_sample_position + (f64)channels * pitch;
        audio_inst->sample_position = fmod(target_sample_position, (f64)sample_count);

        if (target_sample_position >= (f64)sample_count - channels) {
            audio_inst->sample_position = 0.0;
            if (!audio_inst->does_loop) {
                *out_finished = true;
                f++;
                break;
            }
        }
    }
    for (; f < frame_count; f++) {
        out_mono[f] = 0.0f;
    }
}

// Adds 'frame_count' pitch-resampled stereo samples directly into 'mix_out' (already float,
// not yet clamped) for a non-3D instance, scaled by its volume and pan. Same sample_position
// stepping as the mono fetch.
static void fetch_stereo_pitched_samples_additive(SkaAudioInstance* audio_inst, f32* mix_out, ma_uint32 frame_count, bool* out_finished) {
    const int32 channels = audio_inst->source->channels;
    const int32 sample_count = audio_inst->source->sample_count;
    const f64 pitch = audio_inst->source->pitch * (f64)audio_inst->pitch;
    const int16* samples = (const int16*)audio_inst->source->samples;
    // Balance-style pan: center leaves both channels at full volume, so pan = 0 sounds unchanged
    const f32 left_gain = audio_inst->volume * (audio_inst->pan > 0.0f ? 1.0f - audio_inst->pan : 1.0f) / 32768.0f;
    const f32 right_gain = audio_inst->volume * (audio_inst->pan < 0.0f ? 1.0f + audio_inst->pan : 1.0f) / 32768.0f;
    *out_finished = false;

    for (ma_uint32 f = 0; f < frame_count; f++) {
        const f64 start_sample_position = audio_inst->sample_position;
        uint64 cur_index = (uint64)start_sample_position;
        if (channels > 1) {
            cur_index &= ~((uint64)0x01);
        }
        const uint64 next_index = (cur_index + channels) % (uint64)sample_count;
        const int16 sample_left_next = samples[next_index];
        // Mono sources feed both channels (reading next_index + 1 overran the buffer on the last sample)
        const int16 sample_right_next = channels > 1 ? samples[next_index + 1] : sample_left_next;

        mix_out[f * 2] += (f32)sample_left_next * left_gain;
        mix_out[f * 2 + 1] += (f32)sample_right_next * right_gain;

        const f64 target_sample_position = start_sample_position + (f64)channels * pitch;
        audio_inst->sample_position = fmod(target_sample_position, (f64)sample_count);

        if (target_sample_position >= (f64)sample_count - channels) {
            audio_inst->sample_position = 0.0;
            if (!audio_inst->does_loop) {
                *out_finished = true;
                break;
            }
        }
    }
}

// Caller must hold audio_mutex (or the device must be stopped).
static inline void free_audio_instance(SkaAudioInstance* audio_inst) {
    // Scans instead of reading audio_inst->source, which the owner may already have deleted.
    // Without this, get_position on a stopped instance read freed memory.
    for (usize i = 0; i < SKA_MAX_AUDIO_SOURCES; i++) {
        if (audio_source_data[i].last_spawned_instance == audio_inst) {
            audio_source_data[i].last_spawned_instance = NULL;
        }
    }
    if (audio_inst->is_3d) {
        ma_spatializer_uninit(&audio_inst->spatializer, NULL);
    }
    SKA_FREE(audio_inst);
}

void audio_data_callback(ma_device* device, void* output, const void* input, ma_uint32 frame_count) {
    (void)input;
    if (audio_instances->count <= 0) {
        return;
    }

    static f32 mix_buffer_f32[SKA_AUDIO_MAX_FRAME_CHUNK * 2];
    static f32 instance_mono_scratch[SKA_AUDIO_MAX_FRAME_CHUNK];
    static f32 instance_spatialized_f32[SKA_AUDIO_MAX_FRAME_CHUNK * 2];

    pthread_mutex_lock(&audio_mutex);

    const ma_uint32 out_channels = device->playback.channels;
    int16* device_out = (int16*)output;
    ma_uint32 frames_remaining = frame_count;

    while (frames_remaining > 0) {
        const ma_uint32 chunk_frames = frames_remaining > SKA_AUDIO_MAX_FRAME_CHUNK ? SKA_AUDIO_MAX_FRAME_CHUNK : frames_remaining;
        memset(mix_buffer_f32, 0, (usize)chunk_frames * out_channels * sizeof(f32));

        usize removed_instances = 0;
        for (usize i = 0; i < audio_instances->count; i++) {
            SkaAudioInstance* audio_inst = audio_instances->instances[i];
            SKA_ASSERT_FMT(audio_inst != NULL, "audio instance with index %zu is null!", i);
            if (!audio_inst->is_playing) {
                free_audio_instance(audio_inst);
                audio_instances->instances[i] = NULL;
                removed_instances++;
                continue;
            }
            if (audio_inst->is_paused) {
                continue;
            }

            bool instance_finished = false;
            if (audio_inst->is_3d) {
                fetch_mono_pitched_samples(audio_inst, instance_mono_scratch, chunk_frames, &instance_finished);
                ma_spatializer_process_pcm_frames(&audio_inst->spatializer, &audio_listener,
                                                   instance_spatialized_f32, instance_mono_scratch, chunk_frames);
                const f32 volume = audio_inst->volume;
                for (ma_uint32 f = 0; f < chunk_frames * 2; f++) {
                    mix_buffer_f32[f] += instance_spatialized_f32[f] * volume;
                }
            } else {
                fetch_stereo_pitched_samples_additive(audio_inst, mix_buffer_f32, chunk_frames, &instance_finished);
            }

            if (instance_finished) {
                ska_logger_debug("Audio instance with id '%u' is queued for deletion!", audio_inst->id);
                free_audio_instance(audio_inst);
                audio_instances->instances[i] = NULL;
                removed_instances++;
            }
        }

        // Compact out any removed (NULL) slots in place before the next chunk relies on
        // 'count' matching live entries.
        if (removed_instances > 0) {
            usize new_count = 0;
            for (usize i = 0; i < audio_instances->count; i++) {
                if (audio_instances->instances[i] != NULL) {
                    audio_instances->instances[new_count++] = audio_instances->instances[i];
                }
            }
            audio_instances->count = new_count;
        }

        // Convert/clamp once per chunk, from float down to the device's s16 output.
        const ma_uint32 sample_total = chunk_frames * out_channels;
        const f32 output_gain = master_volume * 32768.0f;
        for (ma_uint32 s = 0; s < sample_total; s++) {
            f32 sample = mix_buffer_f32[s] * output_gain;
            if (sample > 32767.0f) {
                sample = 32767.0f;
            } else if (sample < -32768.0f) {
                sample = -32768.0f;
            }
            device_out[s] = (int16)sample;
        }
        device_out += sample_total;
        frames_remaining -= chunk_frames;

        if (audio_instances->count == 0) {
            break;
        }
    }

    pthread_mutex_unlock(&audio_mutex);
}

#endif // #if SKA_AUDIO
