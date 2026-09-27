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
#if SKA_INPUT
#include "seika/input/input.h"
#endif

static SkaWindow main_window = {0};

typedef struct SkaAndroidData {
    struct android_app* app;
    bool window_ready; // true once APP_CMD_INIT_WINDOW has been delivered at least once
#if SKA_RENDERING
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    EGLConfig config;
#endif
} SkaAndroidData;

static SkaAndroidData android_data = {0};

// Runs on the android_main thread via ALooper_pollOnce -> source->process -> this callback
// (native_app_glue's convention, installed onto app->onAppCmd below).
static void handle_app_cmd(struct android_app* app, int32_t cmd) {
    (void)app;
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            android_data.window_ready = true;
            break;
        case APP_CMD_TERM_WINDOW:
        case APP_CMD_DESTROY:
            // Full suspend/resume (recreating the EGL surface after backgrounding) is out of
            // scope for now -- treat both the same as a clean exit request so std_game_run's
            // `while (... && ska_win_is_active(window) ...)` loop (koa/koa/std/game.c) exits
            // cleanly (runs deinit, tears down) instead of continuing to issue GL calls against
            // a dead surface.
            android_data.window_ready = false;
            main_window.is_active = false;
            break;
        default:
            break;
    }
}

void ska_win_android_set_app(struct android_app* app) {
    android_data.app = app;
    app->onAppCmd = handle_app_cmd;
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

// Pumps the looper (blocking while there's nothing else to do) until either the native window
// becomes available or the app is torn down before ever getting one.
static void wait_for_native_window(void) {
    while (!android_data.window_ready && !android_data.app->destroyRequested) {
        int events;
        struct android_poll_source* source;
        const int result = ALooper_pollOnce(-1, NULL, &events, (void**)&source);
        if (result >= 0 && source != NULL) {
            source->process(android_data.app, source);
        }
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
#endif

    main_window = (SkaWindow){
        .title = props.title, .width = width, .height = height, .is_active = true };
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

// Called once per frame from std_game_run's loop (koa/koa/std/game.c) -- must never block, unlike
// wait_for_native_window's initial (-1 timeout) wait.
void ska_win_process_events(SkaWindow* window) {
    (void)window;
    int events;
    struct android_poll_source* source;
    int result;
    while ((result = ALooper_pollOnce(0, NULL, &events, (void**)&source)) >= 0) {
        if (source != NULL) {
            source->process(android_data.app, source);
        }
        if (android_data.app->destroyRequested != 0) {
            main_window.is_active = false;
            return;
        }
    }
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

#endif // #if defined(PLATFORM_ANDROID)
