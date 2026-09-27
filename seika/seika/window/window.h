#pragma once

#include <seika/defines.h>

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#endif

// Represents a single window.  Will eventually support multiple windows.
typedef struct SkaWindow {
    const char* title;
    uint32 width;
    uint32 height;
    bool is_active;
} SkaWindow;

typedef struct SkaCreateWindowProps {
    const char* title;
    int32 pos_x;
    int32 pos_y;
    uint32 width;
    uint32 height;
    bool is_active;
} SkaCreateWindowProps;

#if defined(PLATFORM_WINDOWS)
bool ska_win_init(HINSTANCE h_instance, HINSTANCE h_prev_instance,
                  LPSTR lp_cmd_line, int n_cmd_show);
void ska_win_enable_hscroll_bar(SkaWindow* window, bool enable);
void ska_win_enable_vscroll_bar(SkaWindow* window, bool enable);
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS) || defined(PLATFORM_ANDROID) || defined(PLATFORM_EMSCRIPTEN)
bool ska_win_init(void);
#endif

#if defined(PLATFORM_ANDROID)
// Opaque forward decl -- avoids pulling <android_native_app_glue.h> into every consumer of this
// header. Must be called once by the app's android_main(), before ska_win_init()/std.game.run(),
// so ska_win_init/ska_win_create_window have an android_app to poll the looper and native window
// from. See window_android.c.
struct android_app;
void ska_win_android_set_app(struct android_app* app);
#endif

SkaWindow* ska_win_create_window(SkaCreateWindowProps props);
SkaWindow* ska_win_get_primary_window();
void ska_win_destroy_window(SkaWindow* window);
bool ska_win_is_active(const SkaWindow* window);
void ska_win_process_events(SkaWindow* window);
void ska_win_swap_window(SkaWindow* window);
void ska_win_update_size(SkaWindow* window, int32 width, int32 height);
