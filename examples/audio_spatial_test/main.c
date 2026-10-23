#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <seika/seika_main.h>
#include <seika/window/window.h>
#include <seika/rendering/renderer.h>
#include <seika/rendering/font.h>
#include <seika/input/input.h>
#include <seika/tick.h>
#include <seika/logger.h>
#include <seika/memory.h>
#include <seika/math/math.h>
#include <seika/audio/audio.h>
#include <seika/audio/audio_manager.h>

// Spatial audio sound test:
//   Arrow keys / WASD - move the 3D emitter around the listener (X = left/right pan,
//                        Z = toward/away from the listener = distance attenuation)
//   Space             - fire a one-shot, non-positional 2D "blip" (proves the
//                        non-spatialized mix path still works alongside 3D instances)
//   Escape            - quit
//
// The listener sits at the world origin facing -Z with +Y up and never moves; only the
// emitter (drawn as 'E' on the top-down map, listener is 'L') moves.

static bool is_running = false;
static SkaVector3 emitter_position = {.x = 5.0f, .y = 0.0f, .z = 0.0f};
static SkaAudioInstanceId tone_instance_id = SKA_AUDIO_INVALID_INSTANCE_ID;
static SkaAudioSource* blip_source = NULL;

#define MAP_RANGE 15.0f
#define MAP_CENTER_X 400.0f
#define MAP_CENTER_Y 300.0f
#define MAP_PIXELS_PER_UNIT 15.0f

// Synthesizes a looping mono sine wave at the device's sample rate. duration_seconds is
// chosen by the caller as (whole_cycles / frequency_hz) so the waveform ends exactly where
// it began, avoiding an audible click at the loop point.
static SkaAudioSource* create_tone_source(const char* debug_name, f32 frequency_hz, f32 duration_seconds, f32 amplitude) {
    const uint32 sample_rate = ska_audio_get_wav_sample_rate();
    const int32 frame_count = (int32)(sample_rate * duration_seconds);

    int16_t* samples = (int16_t*)SKA_ALLOC_BYTES(sizeof(int16_t) * (usize)frame_count);
    for (int32 i = 0; i < frame_count; i++) {
        const f32 t = (f32)i / (f32)sample_rate;
        const f32 sample = sinf(2.0f * SKA_PI * frequency_hz * t);
        samples[i] = (int16_t)(sample * amplitude * 32767.0f);
    }

    SkaAudioSource* source = SKA_ALLOC(SkaAudioSource);
    source->file_path = debug_name;
    source->pitch = 1.0;
    source->channels = 1; // mono, required for the 3D spatializer path
    source->sample_rate = (int32)sample_rate;
    source->samples = samples;
    source->sample_count = frame_count;
    ska_audio_manager_register_source(source);
    return source;
}

void gameLoopUpdate(f32 delta_time) {
    static SkaFont* font = NULL;
    if (!font) {
        font = ska_font_create_default_font(18, true);
        if (!font->is_valid) {
            ska_logger_error("Failed to load font for audio_spatial_test example!");
        }
    }

    const f32 move_speed = 8.0f;
    if (ska_input_is_key_pressed(SkaInputKey_KEYBOARD_LEFT, 0) || ska_input_is_key_pressed(SkaInputKey_KEYBOARD_A, 0)) {
        emitter_position.x -= move_speed * delta_time;
    }
    if (ska_input_is_key_pressed(SkaInputKey_KEYBOARD_RIGHT, 0) || ska_input_is_key_pressed(SkaInputKey_KEYBOARD_D, 0)) {
        emitter_position.x += move_speed * delta_time;
    }
    // 'W'/up moves the emitter further along the listener's forward (-Z) axis, i.e. away
    // into the distance; 's'/down brings it back toward and past the listener.
    if (ska_input_is_key_pressed(SkaInputKey_KEYBOARD_UP, 0) || ska_input_is_key_pressed(SkaInputKey_KEYBOARD_W, 0)) {
        emitter_position.z -= move_speed * delta_time;
    }
    if (ska_input_is_key_pressed(SkaInputKey_KEYBOARD_DOWN, 0) || ska_input_is_key_pressed(SkaInputKey_KEYBOARD_S, 0)) {
        emitter_position.z += move_speed * delta_time;
    }
    if (emitter_position.x < -MAP_RANGE) { emitter_position.x = -MAP_RANGE; }
    if (emitter_position.x > MAP_RANGE) { emitter_position.x = MAP_RANGE; }
    if (emitter_position.z < -MAP_RANGE) { emitter_position.z = -MAP_RANGE; }
    if (emitter_position.z > MAP_RANGE) { emitter_position.z = MAP_RANGE; }

    ska_audio_manager_set_instance_position(tone_instance_id, emitter_position);

    if (ska_input_is_key_just_pressed(SkaInputKey_KEYBOARD_SPACE, 0)) {
        ska_audio_manager_play_sound(blip_source, false);
    }
    if (ska_input_is_key_just_released(SkaInputKey_KEYBOARD_ESCAPE, 0)) {
        is_running = false;
    }

    const f32 distance = ska_vector3_length(ska_vector3_sub(emitter_position, SKA_VECTOR3_ZERO));

    ska_renderer_queue_font_draw_call(font, "Spatial Audio Sound Test", 20.0f, 20.0f, 1.0f, SKA_COLOR_WHITE, 0);
    ska_renderer_queue_font_draw_call(font, "Arrows/WASD: move emitter   Space: 2D blip   Esc: quit", 20.0f, 50.0f, 0.8f, SKA_COLOR_WHITE, 0);

    char position_text[128];
    snprintf(position_text, sizeof(position_text), "emitter pos: (%.1f, %.1f)   distance: %.1f", (double)emitter_position.x, (double)emitter_position.z, (double)distance);
    ska_renderer_queue_font_draw_call(font, position_text, 20.0f, 80.0f, 0.8f, SKA_COLOR_WHITE, 0);

    const f32 listener_screen_x = MAP_CENTER_X;
    const f32 listener_screen_y = MAP_CENTER_Y;
    ska_renderer_queue_font_draw_call(font, "L", listener_screen_x, listener_screen_y, 1.2f, SKA_COLOR_GREEN, 0);

    const f32 emitter_screen_x = MAP_CENTER_X + emitter_position.x * MAP_PIXELS_PER_UNIT;
    const f32 emitter_screen_y = MAP_CENTER_Y + emitter_position.z * MAP_PIXELS_PER_UNIT;
    ska_renderer_queue_font_draw_call(font, "E", emitter_screen_x, emitter_screen_y, 1.2f, SKA_COLOR_RED, 0);
}

SKA_MAIN {
    SkaWindow* window = ska_win_create_window((SkaCreateWindowProps){
        .title = "Spatial Audio Sound Test", .pos_x = 100, .pos_y = 100, .width = 800, .height = 600
    });

    ska_renderer_init((SkaRendererInitProps){
        .window_width = 800,
        .window_height = 600,
        .resolution_width = 800,
        .resolution_height = 600,
        .maintain_aspect_ratio = false,
        .init_2d = true,
        .init_props_3d = &(SkaRenderer3DInitProps){
            .fov = 45.0f, .near_plane = 0.1f, .far_plane = 1000.0f}});
    ska_input_initialize();
    ska_logger_set_level(SkaLogLevel_DEBUG);

    ska_audio_initialize();

    SkaAudioSource* tone_source = create_tone_source("synth:tone", 220.0f, 44.0f / 220.0f, 0.5f);
    blip_source = create_tone_source("synth:blip", 660.0f, 6.0f / 660.0f, 0.6f);

    ska_audio_manager_set_listener(
        SKA_VECTOR3_ZERO,
        (SkaVector3){.x = 0.0f, .y = 0.0f, .z = -1.0f},
        SKA_VECTOR3_UP,
        SKA_VECTOR3_ZERO
    );

    SkaAudio3DParams tone_params = SKA_AUDIO_3D_PARAMS_DEFAULT;
    tone_params.min_distance = 1.0f;
    tone_params.max_distance = MAP_RANGE * 1.5f;
    tone_params.rolloff = 1.0f;
    tone_instance_id = ska_audio_manager_play_sound_3d(tone_source, true, emitter_position, tone_params);

    SkaTick* game_loop_tick = ska_tick_create((SkaTickParams){
        .target_fps = 60,
        .target_fixed_fps = 60,
        .update_func = gameLoopUpdate,
    });

    is_running = true;
    while (ska_win_is_active(window) && is_running) {
        ska_win_process_events(window);

        ska_tick_update(game_loop_tick);

        ska_renderer_process_and_flush_batches(&(SkaColor){ .r = 0.05f, .g = 0.05f, .b = 0.1f, .a = 1.0f });

        ska_win_swap_window(window);
        ska_input_new_frame();
    }

    ska_audio_manager_stop_instance(tone_instance_id);
    SKA_FREE(tone_source->samples);
    ska_audio_delete_audio_source_wav(tone_source);
    SKA_FREE(blip_source->samples);
    ska_audio_delete_audio_source_wav(blip_source);
    ska_audio_finalize();

    ska_input_finalize();
    ska_renderer_deinit();
    ska_win_destroy_window(window);
    ska_tick_destroy(game_loop_tick);
    return 0;
}
