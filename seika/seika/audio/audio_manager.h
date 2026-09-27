#pragma once

// TODO: Combine this with audio.h

#if SKA_AUDIO

#ifdef __cplusplus
extern "C" {
#endif

#include "../defines.h"
#include "../math/math.h"

struct SkaAudioSource;

// An id handle for a single playing audio instance, returned by the play_* functions.
// SKA_AUDIO_INVALID_INSTANCE_ID means either the sound failed to play or the instance
// has since finished/been stopped.
typedef uint32 SkaAudioInstanceId;
#define SKA_AUDIO_INVALID_INSTANCE_ID ((SkaAudioInstanceId)0)

// Listener state for 3D/2D positional audio, applied once per frame from the
// camera/player transform. For 2D games, pass a fixed forward/up (e.g. forward =
// (0, 0, -1), up = (0, -1, 0) to match seika's 2D Y-down convention) instead of
// rotating them.
typedef struct SkaAudioListener {
    SkaVector3 position;
    SkaVector3 forward;
    SkaVector3 up;
    SkaVector3 velocity;
} SkaAudioListener;

typedef enum SkaAudioAttenuationModel {
    SkaAudioAttenuationModel_NONE,
    SkaAudioAttenuationModel_INVERSE,
    SkaAudioAttenuationModel_LINEAR,
    SkaAudioAttenuationModel_EXPONENTIAL,
} SkaAudioAttenuationModel;

// Per-instance 3D params. Doppler and directional cones are left out of this first
// pass (doppler_factor = 0 disables doppler); see seika-3d-audio-spec.md.
typedef struct SkaAudio3DParams {
    f32 min_distance;
    f32 max_distance;
    f32 rolloff;
    f32 doppler_factor;
    SkaAudioAttenuationModel attenuation_model;
} SkaAudio3DParams;

#define SKA_AUDIO_3D_PARAMS_DEFAULT SKA_STRUCT_LITERAL(SkaAudio3DParams){ \
    .min_distance = 1.0f, .max_distance = 100.0f, .rolloff = 1.0f, \
    .doppler_factor = 0.0f, .attenuation_model = SkaAudioAttenuationModel_INVERSE }

// --- Audio Manager --- //
bool ska_audio_manager_init();
void ska_audio_manager_finalize();
void ska_audio_manager_register_source(struct SkaAudioSource *source);
void ska_audio_manager_unregister_source(struct SkaAudioSource *source);
// Audio functions

SkaAudioInstanceId ska_audio_manager_play_sound(struct SkaAudioSource* audio_source, bool loops);
SkaAudioInstanceId ska_audio_manager_play_sound2(const char* file_path, bool loops);
void ska_audio_manager_stop_sound(struct SkaAudioSource* audio_source);
void ska_audio_manager_stop_sound2(const char* file_path);
f32 ska_audio_manager_get_position(struct SkaAudioSource* audio_source);
f32 ska_audio_manager_get_position_seconds(struct SkaAudioSource* audio_source);

// --- Spatial (3D/2D positional) Audio --- //
void ska_audio_manager_set_listener(SkaVector3 position, SkaVector3 forward, SkaVector3 up, SkaVector3 velocity);
SkaAudioInstanceId ska_audio_manager_play_sound_3d(struct SkaAudioSource* audio_source, bool loops, SkaVector3 position, SkaAudio3DParams params);
void ska_audio_manager_set_instance_position(SkaAudioInstanceId instance_id, SkaVector3 position);
void ska_audio_manager_set_instance_velocity(SkaAudioInstanceId instance_id, SkaVector3 velocity);
void ska_audio_manager_stop_instance(SkaAudioInstanceId instance_id);
bool ska_audio_manager_is_instance_playing(SkaAudioInstanceId instance_id);

#ifdef __cplusplus
}
#endif

#endif // #if SKA_AUDIO
