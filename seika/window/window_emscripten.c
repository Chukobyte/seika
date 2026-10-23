#include "window.h"

#if defined(PLATFORM_EMSCRIPTEN)

#include <string.h>

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#if SKA_RENDERING
#include "seika/rendering/renderer.h"
#include "seika/rendering/screen_capture.h"
#endif

#include "seika/logger.h"

static SkaWindow main_window = {0};

static char window_title[SKA_WINDOW_TITLE_MAX] = {0};

#if SKA_RENDERING
static EMSCRIPTEN_WEBGL_CONTEXT_HANDLE gl_context = 0;
#endif

// Idempotency guard -- see the matching comment in window_win32.c/window_x11.c/window_android.c.
static bool s_win_system_initialized = false;

bool ska_win_init(void) {
    // No window-system/class registration step exists on the web -- the canvas element already
    // exists synchronously in the default emcc HTML shell (see ska_win_create_window). Nothing to
    // do here beyond the idempotency guard every other backend has.
    s_win_system_initialized = true;
    return true;
}

SkaWindow* ska_win_create_window(SkaCreateWindowProps props) {
    const int32 width = (int32)props.width;
    const int32 height = (int32)props.height;

#if SKA_RENDERING
    emscripten_set_canvas_element_size("#canvas", width, height);

    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.majorVersion = 2;
    attrs.minorVersion = 0;
    attrs.alpha = false; // matches the other backends' opaque-framebuffer assumption

    gl_context = emscripten_webgl_create_context("#canvas", &attrs);
    if (gl_context <= 0) {
        ska_logger_error("emscripten_webgl_create_context failed (WebGL2 unsupported?)");
    }
    emscripten_webgl_make_context_current(gl_context);
#endif

    strncpy(window_title, props.title ? props.title : "", SKA_WINDOW_TITLE_MAX - 1);
    // requestAnimationFrame always paces frames to the display
    main_window = (SkaWindow){
        .title = window_title, .width = width, .height = height, .is_active = true, .vsync_enabled = true };
    ska_win_update_size(&main_window, width, height);
    return &main_window;
}

SkaWindow* ska_win_get_primary_window() {
    return main_window.is_active ? &main_window : NULL;
}

bool ska_win_is_active(const SkaWindow* window) { return window->is_active; }

void ska_win_destroy_window(SkaWindow* window) {
    window->is_active = false;
#if SKA_RENDERING
    if (gl_context > 0) {
        emscripten_webgl_destroy_context(gl_context);
        gl_context = 0;
    }
#endif
}

// True no-op, unlike every native backend: there is no OS event queue to pump here -- the
// browser's own event loop is what invokes the Emscripten main-loop callback in the first place
// (see koa/koa/std/game.c's PLATFORM_EMSCRIPTEN branch, which is what calls this once per frame).
// DOM keyboard/mouse input would be wired via separate emscripten_set_*_callback registrations,
// deliberately deferred -- getting a frame loop rendering in the browser is this milestone's bar.
void ska_win_process_events(SkaWindow* window) {
    (void)window;
}

void ska_win_swap_window(SkaWindow* window) {
#if SKA_RENDERING
    ska_screen_capture_on_frame_end(window->width, window->height);
    // Deliberately no explicit present call here, unlike every other backend's swap/SwapBuffers --
    // WebGL's framebuffer is presented implicitly by the browser compositor at the end of each
    // requestAnimationFrame-driven callback invocation, which is exactly what drives this
    // function's caller. Not a forgotten swap.
#else
    (void)window;
#endif
}

void ska_win_update_size(SkaWindow* window, int32 width, int32 height) {
    window->width = width;
    window->height = height;
#if SKA_RENDERING
    ska_renderer_update_window_size(width, height);
#endif
}

// Sets the page's document.title
void ska_win_set_title(SkaWindow* window, const char* title) {
    strncpy(window_title, title ? title : "", SKA_WINDOW_TITLE_MAX - 1);
    window->title = window_title;
    emscripten_set_window_title(window_title);
}

void ska_win_set_size(SkaWindow* window, int32 width, int32 height) {
    if (width <= 0 || height <= 0) return;
#if SKA_RENDERING
    emscripten_set_canvas_element_size("#canvas", width, height);
#endif
    ska_win_update_size(window, width, height);
}

// deferUntilInEventHandler: browsers only grant fullscreen from a user input handler, so a request
// made from the frame loop is queued until the next key/click
void ska_win_set_fullscreen(SkaWindow* window, bool fullscreen) {
    (void)window;
    if (fullscreen) {
        emscripten_request_fullscreen("#canvas", EM_TRUE);
    } else {
        emscripten_exit_fullscreen();
    }
}

bool ska_win_is_fullscreen(const SkaWindow* window) {
    (void)window;
    EmscriptenFullscreenChangeEvent status;
    return emscripten_get_fullscreen_status(&status) == EMSCRIPTEN_RESULT_SUCCESS && status.isFullscreen;
}

void ska_win_set_vsync(SkaWindow* window, bool enabled) { (void)window; (void)enabled; }

bool ska_win_is_vsync_enabled(const SkaWindow* window) { return window->vsync_enabled; }

#endif // #if defined(PLATFORM_EMSCRIPTEN)
