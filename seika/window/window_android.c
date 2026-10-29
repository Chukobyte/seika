#include "window.h"

#if defined(PLATFORM_ANDROID)

#include <android/native_window.h>
#include <android_native_app_glue.h>

#if SKA_RENDERING
#include <EGL/egl.h>
#include "seika/rendering/renderer.h"
#include "seika/rendering/screen_capture.h"
#endif

#include "seika/logger.h"
#include "seika/assert.h"
#include "seika/file_system.h"
#if SKA_INPUT
#include <math.h>
#include <time.h>
#include <android/configuration.h>
#include <android/input.h>
#include "seika/input/input.h"
#endif

static SkaWindow main_window = {0};

typedef struct SkaAndroidData {
    struct android_app* app;
    bool window_ready; // true once APP_CMD_INIT_WINDOW has been delivered at least once
    bool paused;       // backgrounded: between TERM_WINDOW and the next INIT_WINDOW
#if SKA_RENDERING
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    EGLConfig config;
    EGLint surface_width, surface_height; // last size handed to ska_win_update_size (window->width becomes the viewport's)
#endif
} SkaAndroidData;

static SkaAndroidData android_data = {0};

#if SKA_RENDERING
// INIT_WINDOW can arrive before the window has its real size (a 1x1 surface), and resumes/rotations resize it
static void egl_sync_surface_size(bool force) {
    EGLint width = 0, height = 0;
    if (!eglQuerySurface(android_data.display, android_data.surface, EGL_WIDTH, &width)
        || !eglQuerySurface(android_data.display, android_data.surface, EGL_HEIGHT, &height)) return;
    if (!force && width == android_data.surface_width && height == android_data.surface_height) return;
    android_data.surface_width = width;
    android_data.surface_height = height;
    ska_win_update_size(&main_window, (int32)width, (int32)height);
}

// Backgrounding destroys the native window; only the surface goes, the context (textures, fonts, shaders) is kept
static void egl_release_surface(void) {
    if (android_data.display == EGL_NO_DISPLAY) return;
    eglMakeCurrent(android_data.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (android_data.surface != EGL_NO_SURFACE) {
        eglDestroySurface(android_data.display, android_data.surface);
        android_data.surface = EGL_NO_SURFACE;
    }
}

// Binds the kept context to a surface on the new native window; false if the context didn't survive
static bool egl_restore_surface(void) {
    EGLint native_visual_id = 0;
    eglGetConfigAttrib(android_data.display, android_data.config, EGL_NATIVE_VISUAL_ID, &native_visual_id);
    ANativeWindow_setBuffersGeometry(android_data.app->window, 0, 0, native_visual_id);
    android_data.surface = eglCreateWindowSurface(android_data.display, android_data.config, android_data.app->window, NULL);
    if (android_data.surface == EGL_NO_SURFACE) {
        ska_logger_error("resume: eglCreateWindowSurface failed (0x%x)", eglGetError());
        return false;
    }
    if (!eglMakeCurrent(android_data.display, android_data.surface, android_data.surface, android_data.context)) {
        ska_logger_error("resume: eglMakeCurrent failed (0x%x)", eglGetError());
        return false;
    }
    egl_sync_surface_size(true);
    // swap interval belongs to the surface
    eglSwapInterval(android_data.display, main_window.vsync_enabled ? 1 : 0);
    return true;
}
#endif

// Runs on the android_main thread via ALooper_pollOnce -> source->process -> this callback
// (native_app_glue's convention, installed onto app->onAppCmd below).
static void handle_app_cmd(struct android_app* app, int32_t cmd) {
    (void)app;
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            android_data.window_ready = true;
            if (android_data.paused) {
                android_data.paused = false;
#if SKA_RENDERING
                // without a context there's nothing to draw with, so end the game cleanly
                if (!egl_restore_surface()) {
                    main_window.is_active = false;
                }
#endif
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // app->window is still valid here; the game loop blocks in ska_win_process_events until INIT_WINDOW
            android_data.window_ready = false;
            android_data.paused = main_window.is_active;
#if SKA_RENDERING
            egl_release_surface();
#endif
            break;
        case APP_CMD_DESTROY:
            android_data.window_ready = false;
            android_data.paused = false;
            main_window.is_active = false;
            break;
        default:
            break;
    }
}

#if SKA_INPUT
// Touch drives the mouse: a tap is a left click (sent on lift), a drag scrolls like the wheel,
// and a long press is a right click. Press and release go out in separate frames so just_pressed is seen.
#define TOUCH_SLOP_DP 10.0f
#define TOUCH_SCROLL_STEP_DP 40.0f
#define TOUCH_LONG_PRESS_NS 500000000LL

typedef struct SkaAndroidTouch {
    bool down;
    bool dragging;
    bool long_pressed;
    f32 start_x, start_y;
    f32 scroll_anchor_y;
    int64_t down_time_ns;
    bool release_left;   // sent at the start of the next frame
    bool release_right;
} SkaAndroidTouch;

static SkaAndroidTouch touch = {0};

static int64_t monotonic_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

static f32 dp_to_px(f32 dp) {
    const int32_t density = android_data.app->config ? AConfiguration_getDensity(android_data.app->config) : 160;
    return dp * (f32)(density > 0 ? density : 160) / 160.0f;
}

static void touch_move_mouse(f32 x, f32 y) {
    ska_input_register_mouse_move_event((SkaVector2i){ (int32)x, (int32)y });
}

static void handle_touch(const AInputEvent* event) {
    const int32_t action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
    const f32 x = AMotionEvent_getX(event, 0);
    const f32 y = AMotionEvent_getY(event, 0);
    switch (action) {
        case AMOTION_EVENT_ACTION_DOWN:
            touch = (SkaAndroidTouch){ .down = true, .start_x = x, .start_y = y, .scroll_anchor_y = y,
                .down_time_ns = monotonic_ns(), .release_left = touch.release_left, .release_right = touch.release_right };
            touch_move_mouse(x, y);
            break;
        case AMOTION_EVENT_ACTION_MOVE: {
            if (!touch.down) break;
            touch_move_mouse(x, y);
            const f32 slop = dp_to_px(TOUCH_SLOP_DP);
            if (!touch.dragging && !touch.long_pressed && (fabsf(x - touch.start_x) > slop || fabsf(y - touch.start_y) > slop)) {
                touch.dragging = true;
                touch.scroll_anchor_y = y;
            }
            if (touch.dragging) {
                // finger up = content moves up = wheel down (negative)
                const f32 step = dp_to_px(TOUCH_SCROLL_STEP_DP);
                const f32 dy = y - touch.scroll_anchor_y;
                if (fabsf(dy) >= step) {
                    const f32 notches = (f32)(int32)(dy / step);
                    ska_input_register_mouse_scroll_event((SkaVector2){ 0.0f, notches });
                    touch.scroll_anchor_y += notches * step;
                }
            }
            break;
        }
        case AMOTION_EVENT_ACTION_UP:
            // after a drag nothing should stay hovered where the finger lifted
            touch_move_mouse(touch.dragging ? -10000.0f : x, touch.dragging ? -10000.0f : y);
            if (touch.down && !touch.dragging && !touch.long_pressed) {
                ska_input_register_input_event3(SkaInputSourceType_MOUSE, SkaInputKey_MOUSE_BUTTON_LEFT, SkaInputTriggerType_PRESSED);
                touch.release_left = true;
            }
            touch.down = false;
            break;
        case AMOTION_EVENT_ACTION_CANCEL:
            touch.down = false;
            break;
        default:
            break;
    }
}

static SkaInputKey translate_keycode(int32_t keycode) {
    if (keycode >= AKEYCODE_A && keycode <= AKEYCODE_Z) return (SkaInputKey)(SkaInputKey_KEYBOARD_A + (keycode - AKEYCODE_A));
    if (keycode >= AKEYCODE_0 && keycode <= AKEYCODE_9) return (SkaInputKey)(SkaInputKey_KEYBOARD_NUM_0 + (keycode - AKEYCODE_0));
    switch (keycode) {
        case AKEYCODE_BACK:
        case AKEYCODE_ESCAPE: return SkaInputKey_KEYBOARD_ESCAPE;
        case AKEYCODE_ENTER: return SkaInputKey_KEYBOARD_RETURN;
        case AKEYCODE_DEL: return SkaInputKey_KEYBOARD_BACKSPACE;
        case AKEYCODE_SPACE: return SkaInputKey_KEYBOARD_SPACE;
        case AKEYCODE_MINUS: return SkaInputKey_KEYBOARD_MINUS;
        case AKEYCODE_PERIOD: return SkaInputKey_KEYBOARD_PERIOD;
        case AKEYCODE_APOSTROPHE: return SkaInputKey_KEYBOARD_QUOTE;
        case AKEYCODE_SHIFT_LEFT: return SkaInputKey_KEYBOARD_LEFT_SHIFT;
        case AKEYCODE_SHIFT_RIGHT: return SkaInputKey_KEYBOARD_RIGHT_SHIFT;
        case AKEYCODE_DPAD_UP: return SkaInputKey_KEYBOARD_UP;
        case AKEYCODE_DPAD_DOWN: return SkaInputKey_KEYBOARD_DOWN;
        case AKEYCODE_DPAD_LEFT: return SkaInputKey_KEYBOARD_LEFT;
        case AKEYCODE_DPAD_RIGHT: return SkaInputKey_KEYBOARD_RIGHT;
        default: return SkaInputKey_INVALID;
    }
}

// Returning 1 consumes the event (Back must be consumed or the activity finishes)
static int32_t handle_input(struct android_app* app, AInputEvent* event) {
    (void)app;
    const int32_t type = AInputEvent_getType(event);
    if (type == AINPUT_EVENT_TYPE_MOTION) {
        handle_touch(event);
        return 1;
    }
    if (type == AINPUT_EVENT_TYPE_KEY) {
        const SkaInputKey key = translate_keycode(AKeyEvent_getKeyCode(event));
        if (key == SkaInputKey_INVALID) return 0;
        const int32_t action = AKeyEvent_getAction(event);
        if (action == AKEY_EVENT_ACTION_DOWN && AKeyEvent_getRepeatCount(event) == 0) {
            ska_input_register_input_event3(SkaInputSourceType_KEYBOARD, key, SkaInputTriggerType_PRESSED);
        } else if (action == AKEY_EVENT_ACTION_UP) {
            ska_input_register_input_event3(SkaInputSourceType_KEYBOARD, key, SkaInputTriggerType_RELEASED);
        }
        return 1;
    }
    return 0;
}

// Releases last frame's synthesized clicks and fires a long press once the finger has held still long enough.
static void update_touch(void) {
    if (touch.release_left) {
        ska_input_register_input_event3(SkaInputSourceType_MOUSE, SkaInputKey_MOUSE_BUTTON_LEFT, SkaInputTriggerType_RELEASED);
        touch.release_left = false;
        // the tap's frame has seen the click; a lifted finger hovers nothing
        if (!touch.down) {
            touch_move_mouse(-10000.0f, -10000.0f);
        }
    }
    if (touch.release_right) {
        ska_input_register_input_event3(SkaInputSourceType_MOUSE, SkaInputKey_MOUSE_BUTTON_RIGHT, SkaInputTriggerType_RELEASED);
        touch.release_right = false;
    }
    if (touch.down && !touch.dragging && !touch.long_pressed && monotonic_ns() - touch.down_time_ns >= TOUCH_LONG_PRESS_NS) {
        touch.long_pressed = true;
        ska_input_register_input_event3(SkaInputSourceType_MOUSE, SkaInputKey_MOUSE_BUTTON_RIGHT, SkaInputTriggerType_PRESSED);
        touch.release_right = true;
    }
}
#endif // SKA_INPUT

void ska_win_android_set_app(struct android_app* app) {
    android_data.app = app;
    app->onAppCmd = handle_app_cmd;
#if SKA_INPUT
    app->onInputEvent = handle_input;
#endif
    if (app->activity && app->activity->internalDataPath) {
        ska_fs_set_user_data_root(app->activity->internalDataPath);
    }
}

// Idempotency guard -- see the matching comment in window_win32.c/window_x11.c.
static bool s_win_system_initialized = false;

bool ska_win_init(void) {
    if (s_win_system_initialized) return true;
    if (!android_data.app) {
        ska_logger_error("ska_win_init: ska_win_android_set_app must be called first");
        return false;
    }
    s_win_system_initialized = true;
    return true;
}

// Processes one looper event (timeout -1 blocks); false when there was nothing to process
static bool pump_event(int timeout_ms) {
    int events;
    struct android_poll_source* source;
    if (ALooper_pollOnce(timeout_ms, NULL, &events, (void**)&source) < 0) return false;
    if (source != NULL) {
        source->process(android_data.app, source);
    }
    return true;
}

// Pumps the looper (blocking while there's nothing else to do) until either the native window
// becomes available or the app is torn down before ever getting one.
static void wait_for_native_window(void) {
    while (!android_data.window_ready && !android_data.app->destroyRequested) {
        pump_event(-1);
    }
}

#if SKA_RENDERING
static bool egl_init(void) {
    android_data.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (android_data.display == EGL_NO_DISPLAY) {
        ska_logger_error("eglGetDisplay failed");
        return false;
    }
    if (!eglInitialize(android_data.display, NULL, NULL)) {
        ska_logger_error("eglInitialize failed");
        return false;
    }

    // EGL_DEPTH_SIZE 24 / EGL_STENCIL_SIZE 8 is the widely-supported combo across Android GPUs --
    // window_x11.c's GLX config requests 32/8 to match win32's PFD, but 32-bit depth is rare on
    // mobile EGL configs, so this intentionally diverges.
    const EGLint config_attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };
    EGLint num_configs = 0;
    if (!eglChooseConfig(android_data.display, config_attribs, &android_data.config, 1, &num_configs) || num_configs < 1) {
        ska_logger_error("eglChooseConfig failed to find a matching config");
        return false;
    }

    EGLint native_visual_id = 0;
    eglGetConfigAttrib(android_data.display, android_data.config, EGL_NATIVE_VISUAL_ID, &native_visual_id);
    ANativeWindow_setBuffersGeometry(android_data.app->window, 0, 0, native_visual_id);

    android_data.surface = eglCreateWindowSurface(android_data.display, android_data.config, android_data.app->window, NULL);
    if (android_data.surface == EGL_NO_SURFACE) {
        ska_logger_error("eglCreateWindowSurface failed");
        return false;
    }

    const EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    android_data.context = eglCreateContext(android_data.display, android_data.config, EGL_NO_CONTEXT, context_attribs);
    if (android_data.context == EGL_NO_CONTEXT) {
        ska_logger_error("eglCreateContext failed");
        return false;
    }

    if (!eglMakeCurrent(android_data.display, android_data.surface, android_data.surface, android_data.context)) {
        ska_logger_error("eglMakeCurrent failed");
        return false;
    }
    return true;
}
#endif // SKA_RENDERING

SkaWindow* ska_win_create_window(SkaCreateWindowProps props) {
    wait_for_native_window();

    int32 width = (int32)props.width;
    int32 height = (int32)props.height;

#if SKA_RENDERING
    SKA_ASSERT_REL_FMT(egl_init(), "Failed to initialize EGL");
    // The requested width/height are desktop-window sizing hints -- Android native windows are
    // fullscreen and report their actual pixel size only once the EGL surface exists.
    EGLint surface_width = width;
    EGLint surface_height = height;
    eglQuerySurface(android_data.display, android_data.surface, EGL_WIDTH, &surface_width);
    eglQuerySurface(android_data.display, android_data.surface, EGL_HEIGHT, &surface_height);
    width = (int32)surface_width;
    height = (int32)surface_height;
    android_data.surface_width = surface_width;
    android_data.surface_height = surface_height;
#endif

    // Native windows are always fullscreen; eglSwapInterval defaults to 1
    main_window = (SkaWindow){
        .title = "", .width = width, .height = height, .is_active = true,
        .is_fullscreen = true, .vsync_enabled = true };
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
    if (android_data.display != EGL_NO_DISPLAY) {
        eglMakeCurrent(android_data.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (android_data.context != EGL_NO_CONTEXT) eglDestroyContext(android_data.display, android_data.context);
        if (android_data.surface != EGL_NO_SURFACE) eglDestroySurface(android_data.display, android_data.surface);
        eglTerminate(android_data.display);
    }
    android_data.display = EGL_NO_DISPLAY;
    android_data.surface = EGL_NO_SURFACE;
    android_data.context = EGL_NO_CONTEXT;
#endif
}

// Called once per frame from std_game_run's loop (koa/koa/std/game.c). Only blocks while backgrounded,
// so the game loop stalls here (no GL calls without a surface) until the window comes back.
void ska_win_process_events(SkaWindow* window) {
    (void)window;
#if SKA_INPUT
    update_touch();
#endif
    while (!android_data.app->destroyRequested && pump_event(0)) {}
    if (android_data.paused) {
        while (android_data.paused && !android_data.app->destroyRequested) {
            pump_event(-1);
        }
#if SKA_INPUT
        // a finger that was down when the app left never sends its UP
        touch.down = false;
        touch_move_mouse(-10000.0f, -10000.0f);
#endif
    }
    if (android_data.app->destroyRequested != 0) {
        main_window.is_active = false;
        return;
    }
#if SKA_RENDERING
    if (main_window.is_active && android_data.surface != EGL_NO_SURFACE) {
        egl_sync_surface_size(false);
    }
#endif
}

void ska_win_swap_window(SkaWindow* window) {
#if SKA_RENDERING
    ska_screen_capture_on_frame_end(window->width, window->height);
    eglSwapBuffers(android_data.display, android_data.surface);
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

// No title bar, and the OS owns the window's size and fullscreen state
void ska_win_set_title(SkaWindow* window, const char* title) { (void)window; (void)title; }
void ska_win_set_size(SkaWindow* window, int32 width, int32 height) { (void)window; (void)width; (void)height; }
void ska_win_set_fullscreen(SkaWindow* window, bool fullscreen) { (void)window; (void)fullscreen; }
bool ska_win_is_fullscreen(const SkaWindow* window) { return window->is_fullscreen; }

void ska_win_set_vsync(SkaWindow* window, bool enabled) {
#if SKA_RENDERING
    if (eglSwapInterval(android_data.display, enabled ? 1 : 0)) {
        window->vsync_enabled = enabled;
    }
#else
    (void)window; (void)enabled;
#endif
}

bool ska_win_is_vsync_enabled(const SkaWindow* window) { return window->vsync_enabled; }

#endif // #if defined(PLATFORM_ANDROID)
