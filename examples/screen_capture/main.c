#include <math.h>
#include <seika/seika_main.h>
#include <seika/window/window.h>
#include <seika/rendering/renderer.h>
#include <seika/rendering/font.h>
#include <seika/rendering/screen_capture.h>
#include <seika/input/input.h>
#include <seika/tick.h>
#include <seika/logger.h>

// Demonstrates seika's screenshot + gif capture API:
//   F9  - take a screenshot (screen_capture_example_shot.png)
//   F10 - toggle gif recording (screen_capture_example_clip.gif)
static bool is_running = false;
static bool is_recording = false;

void gameLoopUpdate(f32 delta_time) {
    static SkaFont* font = NULL;
    if (!font) {
        font = ska_font_create_default_font(20, true);
        if (!font->is_valid) {
          ska_logger_error("Failed to load font for screen_capture example!");
        }
    }

    // Bouncing label so a gif recording actually shows motion.
    static f32 elapsed = 0.0f;
    elapsed += delta_time;
    const f32 x = 300.0f + sinf(elapsed) * 200.0f;
    const f32 y = 250.0f + cosf(elapsed * 1.3f) * 100.0f;
    ska_renderer_queue_font_draw_call(font, "seika", x, y, 1.5f, SKA_COLOR_WHITE, 0);

    ska_renderer_queue_font_draw_call(font, "F9: screenshot   F10: toggle gif recording", 20.0f, 20.0f, 1.0f, SKA_COLOR_WHITE, 0);
    if (is_recording) {
        ska_renderer_queue_font_draw_call(font, "RECORDING...", 20.0f, 50.0f, 1.0f, SKA_COLOR_RED, 0);
    }

    if (ska_input_is_key_just_pressed(SkaInputKey_KEYBOARD_F9, 0)) {
        ska_logger_debug("Taking screenshot -> screen_capture_example_shot.png");
        ska_screenshot_capture("screen_capture_example_shot.png");
    }

    if (ska_input_is_key_just_pressed(SkaInputKey_KEYBOARD_F10, 0)) {
        if (!is_recording) {
            ska_logger_debug("Starting gif recording...");
            ska_gif_recorder_start(
                (SkaGifRecorderConfig){.max_frames = 300,
                                       .sample_every_n_frames = 2,
                                       .centi_seconds_per_frame = 4});
            is_recording = true;
        } else {
            ska_logger_debug("Stopping gif recording -> screen_capture_example_clip.gif");
            ska_gif_recorder_stop("screen_capture_example_clip.gif");
            is_recording = false;
        }
    }

    if (ska_input_is_key_just_released(SkaInputKey_KEYBOARD_ESCAPE, 0)) {
        is_running = false;
    }
}

SKA_MAIN {
    SkaWindow* window = ska_win_create_window((SkaCreateWindowProps){
        .title = "Screen Capture Example", .pos_x = 100, .pos_y = 100, .width = 800, .height = 600
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

    SkaTick *game_loop_tick = ska_tick_create((SkaTickParams){
        .target_fps = 60,
        .target_fixed_fps = 60,
        .update_func = gameLoopUpdate,
    });

    is_running = true;
    while (ska_win_is_active(window) && is_running) {
        ska_win_process_events(window);

        ska_tick_update(game_loop_tick);

        ska_renderer_process_and_flush_batches(&(SkaColor){ .r = 0.1f, .g = 0.1f, .b = 0.15f, .a = 1.0f });

        ska_win_swap_window(window);
        ska_input_new_frame();
    }

    if (is_recording) {
        ska_gif_recorder_stop("screen_capture_example_clip.gif");
    }

    ska_input_finalize();
    ska_renderer_deinit();
    ska_win_destroy_window(window);
    ska_tick_destroy(game_loop_tick);
    return 0;
}
