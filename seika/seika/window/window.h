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
    bool isActive;
} SkaWindow;

typedef struct SkaCreateWindowProps {
    const char* title;
    int32 posX;
    int32 posY;
    uint32 width;
    uint32 height;
    bool isActive;
} SkaCreateWindowProps;

#if defined(PLATFORM_WINDOWS)
bool ska_win_init(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
void ska_win_enable_hscroll_bar(SkaWindow* window, bool enable);
void ska_win_enable_vscroll_bar(SkaWindow* window, bool enable);
#endif

SkaWindow* ska_win_create_window(SkaCreateWindowProps props);
SkaWindow* ska_win_get_primary_window();
void ska_win_destroy_window(SkaWindow* window);
bool ska_win_is_active(const SkaWindow* window);
void ska_win_process_events(SkaWindow* window);
void ska_win_swap_window(SkaWindow* window);
void ska_win_update_size(SkaWindow* window, int32 width, int32 height);
