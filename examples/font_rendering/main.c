#include <seika/seika_main.h>
#include <seika/window/window.h>
#include <seika/rendering/renderer.h>
#include <seika/rendering/font.h>
#include <seika/input/input.h>
#include <seika/logger.h>

// Visual check for text positioning: draws labeled text at a handful of fixed
// positions, including (0,0), so a top-left-clipped glyph is immediately obvious.
static bool is_running = false;

void gameLoopUpdate(f32 delta_time) {
    (void) delta_time;

    static SkaFont* font = NULL;
    if (!font) {
        font = ska_font_create_default_font(16, true);
        if (!font->is_valid) {
          ska_logger_error("Failed to load font for font_rendering example!");
        }
    }

    ska_renderer_queue_font_draw_call(font, "0,0", 0.0f, 0.0f, 1.0f, SKA_COLOR_WHITE, 0);
    ska_renderer_queue_font_draw_call(font, "0,100", 0.0f, 100.0f, 1.0f, SKA_COLOR_WHITE, 0);
    ska_renderer_queue_font_draw_call(font, "100,100", 100.0f, 100.0f, 1.0f, SKA_COLOR_WHITE, 0);
    ska_renderer_queue_font_draw_call(font, "600,550", 600.0f, 550.0f, 1.0f, SKA_COLOR_WHITE, 0);

    if (ska_input_is_key_just_released(SkaInputKey_KEYBOARD_ESCAPE, 0)) {
        is_running = false;
    }
}

SKA_MAIN {
    SkaWindow* window = ska_win_create_window((SkaCreateWindowProps){
        .title = "Font Rendering Example", .pos_x = 100, .pos_y = 100, .width = 800, .height = 600
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

    is_running = true;
    while (ska_win_is_active(window) && is_running) {
        ska_win_process_events(window);

        gameLoopUpdate(0.0f);

        ska_renderer_process_and_flush_batches(&(SkaColor){ .r = 0.1f, .g = 0.1f, .b = 0.1f, .a = 1.0f });

        ska_win_swap_window(window);
        ska_input_new_frame();
    }
    ska_input_finalize();
    ska_renderer_deinit();
    ska_win_destroy_window(window);
    return 0;
}
