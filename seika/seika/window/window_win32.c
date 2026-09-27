#include "window.h"

#if defined(PLATFORM_WINDOWS)

#include <stdio.h>
#include <io.h> // For _open_osfhandle
#include <fcntl.h> // For _O_TEXT
#include <wingdi.h>
#include "seika/logger.h"
#include "seika/input/input_win32.h"
#include "seika/input/input_directinput.h"

#if SKA_RENDERING
#include "seika/rendering/renderer.h"
#include "seika/rendering/screen_capture.h"
#endif

static SkaWindow main_window = {0};

typedef struct ZoWin32Data {
    HINSTANCE h_instance;
    HWND hwnd; // Should attach to window data if we ever support multiple windows
    HDC hdc;
    int n_cmd_show;
} ZoWin32Data;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param);

// Define the name of the window class
static const char class_name[] = "Zo Window Class";

static ZoWin32Data win32_data = {0};

#if SKA_RENDERING
static bool is_open_gl_initialized = false;

// OPENGL
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0X2092
#define WGL_CONTEXT_FLAGS_ARB 0X2094
#define WGL_CONTEXT_COREPROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
typedef const char* (WINAPI* PFNWGLGETEXTENSIONSSTRINGEXTPROC)(void);
typedef BOOL (WINAPI* PFNWGLSWAPINTERVALEXTPROC)(int);
typedef int (WINAPI* PFNWGLGETSWAPINTERVALEXTPROC) (void);
typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC h_dc, HGLRC h_share_context, const int* attrib_list);

static void opengl_init(HWND hwnd) {
    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;

    //https://en.wikipedia.org/wiki/Stencil_buffer#:~:text=A%20stencil%20buffer%20is%20an,RAM%20of%20the%20graphics%20hardware.
    pfd.cDepthBits = 32;
    pfd.cStencilBits = 8;

    HDC hdc = GetDC(hwnd);
    win32_data.hdc = hdc;
    int pixel_format = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixel_format, &pfd);

    HGLRC temp_rc = wglCreateContext(hdc);
    wglMakeCurrent(hdc, temp_rc);
    PFNWGLCREATECONTEXTATTRIBSARBPROC wgl_create_context_attribs_arb = NULL;
    wgl_create_context_attribs_arb = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    const int attrib_list[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_FLAGS_ARB, 0,
        WGL_CONTEXT_PROFILE_MASK_ARB,
        WGL_CONTEXT_COREPROFILE_BIT_ARB, 0,
        0 // Terminator
    };

    HGLRC hglrc = wgl_create_context_attribs_arb(hdc, 0, attrib_list);
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(temp_rc);
    wglMakeCurrent(hdc, hglrc);

    if (!gladLoadGL()) {
        ska_logger_error("Could not initialize GLAD!");
    }
    else {
      ska_logger_debug("OpenGL Version %d", GLVersion.major);
    }

    PFNWGLGETEXTENSIONSSTRINGEXTPROC wgl_get_extensions_string_ext = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
    bool swap_control_supported = strstr(wgl_get_extensions_string_ext(), "WGL_EXT_swap_control") != 0;
    //https://www.khronos.org/opengl/wiki/Swap_Interval
    int vsynch = 0;

    if (swap_control_supported) {
        PFNWGLSWAPINTERVALEXTPROC wgl_swap_internal_ext = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
        PFNWGLGETSWAPINTERVALEXTPROC wgl_get_swap_interval_ext = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
        if (wgl_swap_internal_ext(1)) {
            ska_logger_debug("VSynch enabled");
        }
        else {
            ska_logger_error("Could not enable VSynch");
        }
    }
    else {
        ska_logger_error("WGL_EXT_swap_control not supported");
    }
    is_open_gl_initialized = true;
}
#endif // SKA_RENDERING

static void enable_console() {
    // If stdout is already a valid handle -- an inherited console, or redirected to a file/pipe
    // (any automated/CI test run, `foo.exe > log.txt`, `foo.exe | grep ...`) -- leave it alone.
    // AttachConsole/AllocConsole + freopen_s("CONOUT$") unconditionally replaced stdout with a
    // fresh console handle regardless, silently discarding whatever it was already pointed at;
    // under redirection that meant every printf after this ran (koa_std_unit_tests.exe's output
    // after the first test to register std.game, for example) vanished with no error, no crash --
    // the process ran to real completion, just with a stdout no one could read. Only a true
    // GUI-subsystem process with no console and no redirection (e.g. a game launched by double-
    // click) has no stdout handle at all here, which is the one case that still needs this.
    HANDLE existing_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (existing_stdout != NULL && existing_stdout != INVALID_HANDLE_VALUE) {
        return;
    }
    if (AttachConsole(ATTACH_PARENT_PROCESS) == 0) { // Try to attach to parent console
        AllocConsole(); // If no parent console, allocate a new one
    }
    FILE* p_file;
    freopen_s(&p_file, "CONOUT$", "w", stdout);
}

// Idempotency guard -- SKA_MAIN's wrapper (seika_main.c) already calls this before a windowed
// host's entry point runs; a host that also embeds koa's std.game (koa/koa/std/game.c) calls it
// again defensively so it works from hosts that never go through SKA_MAIN at all (e.g. koa's
// standalone runtime). A second RegisterClass with the same class name would otherwise fail.
static bool s_win_system_initialized = false;

bool ska_win_init(HINSTANCE h_instance, HINSTANCE h_prev_instance, LPSTR lp_cmd_line, int n_cmd_show) {
    if (s_win_system_initialized) return true;

    // Register the Window Class
    WNDCLASS wc = {0};
    wc.lpfnWndProc   = WindowProc;        // Pointer to the window procedure
    wc.hInstance     = h_instance;         // Handle to the application instance
    wc.lpszClassName = class_name;        // Name of the window class
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW); // Default cursor
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);  // Background color

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }
    win32_data = (ZoWin32Data){
        .h_instance = h_instance, .hdc = NULL, .n_cmd_show = n_cmd_show};
    enable_console();

#if SKA_INPUT
    ska_input_directinput_init(h_instance);
#endif

    s_win_system_initialized = true;
    return true;
}

SkaWindow* ska_win_create_window(SkaCreateWindowProps props) {
    const char* title = props.title;
    const int32 pos_x = props.pos_x;
    const int32 pos_y = props.pos_y;
    const int32 width = (int32)props.width;
    const int32 height = (int32)props.height;

    // width/height above are meant to be the CLIENT (drawable) area, matching what
    // callers pass to ska_renderer_init as the resolution/window size. CreateWindowEx
    // instead treats its size params as the OUTER window rect, so without this
    // adjustment the client area ends up smaller than requested (by the title bar and
    // borders) until a later WM_SIZE (e.g. from maximizing) reports the real size.
    RECT window_rect = { 0, 0, width, height };
    AdjustWindowRectEx(&window_rect, WS_OVERLAPPEDWINDOW, FALSE, 0);
    const int32 outer_width = window_rect.right - window_rect.left;
    const int32 outer_height = window_rect.bottom - window_rect.top;

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles
        class_name,                     // Window class
        title,                          // Window title
        WS_OVERLAPPEDWINDOW,            // Window style
        // Size and position
        pos_x, pos_y, outer_width, outer_height,
        NULL,                           // Parent window
        NULL,                           // Menu
        win32_data.h_instance,            // Instance handle
        NULL                            // Additional application data
    );

    if (hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    win32_data.hwnd = hwnd;
    ShowWindow(hwnd, win32_data.n_cmd_show);
    // Without this, the window can be created and shown without ever receiving real OS keyboard
    // focus (e.g. after enable_console()'s AllocConsole() already consumed the process's foreground
    // grant), so WM_KEYDOWN/WM_KEYUP never fire for it no matter what the user clicks on.
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

#if SKA_RENDERING
    opengl_init(hwnd);
#endif

#if SKA_INPUT
    ska_input_directinput_enumerate(hwnd);
#endif

    main_window = (SkaWindow){
        .title = title, .width = width, .height = height, .is_active = true};
    ska_win_update_size(&main_window, width, height);
    return &main_window;
}

SkaWindow* ska_win_get_primary_window() {
  return main_window.is_active ? &main_window : NULL;
}

bool ska_win_is_active(const SkaWindow *window) { return window->is_active; }

void ska_win_destroy_window(SkaWindow* window) {
  window->is_active = false;
#if SKA_RENDERING
    is_open_gl_initialized = false;
#endif
}

void ska_win_process_events(SkaWindow* window) {
    MSG msg = {0};
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

#if SKA_INPUT
    // DirectInput isn't message-driven (no WM_* for gamepad state), so it needs an explicit
    // per-frame poll rather than living inside the PeekMessage loop above.
    ska_input_directinput_poll_gamepads();
#endif
}

void ska_win_swap_window(SkaWindow* window) {
#if SKA_RENDERING
    ska_screen_capture_on_frame_end(window->width, window->height);
    SwapBuffers(win32_data.hdc);
#endif
}

void ska_win_update_size(SkaWindow* window, int32 width, int32 height) {
    window->width = width;
    window->height = height;
#if SKA_RENDERING
    ska_renderer_update_window_size(width, height);
#endif
}

void ska_win_enable_hscroll_bar(SkaWindow* window, bool enable) {
    LONG style = GetWindowLong(win32_data.hwnd, GWL_STYLE);
    if (enable) {
        style |= WS_HSCROLL;
    } else {
        style &= ~WS_HSCROLL;
    }
    SetWindowLong(win32_data.hwnd, GWL_STYLE, style);
    SetWindowPos(win32_data.hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

void ska_win_enable_vscroll_bar(SkaWindow* window, bool enable) {
    LONG style = GetWindowLong(win32_data.hwnd, GWL_STYLE);
    if (enable) {
        style |= WS_VSCROLL;
    } else {
        style &= ~WS_VSCROLL;
    }
    SetWindowLong(win32_data.hwnd, GWL_STYLE, style);
    SetWindowPos(win32_data.hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param) {
    switch (u_msg) {
        case WM_DESTROY: {
          main_window.is_active = false;
          PostQuitMessage(0);
          return 0;
        }
        case WM_SIZE: {
#if SKA_RENDERING
            if (is_open_gl_initialized) {
                const int32 width = LOWORD(l_param);
                const int32 height = HIWORD(l_param);
                ska_win_update_size(&main_window, width, height); // TODO: Using mainWindow here maybe fix later?
                main_window.width = width;
                main_window.height = height;
            }
#endif
            return 0;
        }
#if SKA_INPUT
        case WM_KEYDOWN: {
            const uint32 vk_code = (uint32)w_param;
            const SkaInputKey translated_key = ska_input_win32_vkcode_to_keycode(vk_code);
            ska_input_register_input_event3(SkaInputSourceType_KEYBOARD, translated_key, SkaInputTriggerType_PRESSED);
            return 0;
        }
        case WM_KEYUP: {
            const uint32 vk_code = (uint32)w_param;
            const SkaInputKey translated_key = ska_input_win32_vkcode_to_keycode(vk_code);
            ska_input_register_input_event3(SkaInputSourceType_KEYBOARD, translated_key, SkaInputTriggerType_RELEASED);
            return 0;
        }
        case WM_LBUTTONDOWN: {
            ska_input_register_input_event3(SkaInputSourceType_MOUSE, SkaInputKey_MOUSE_BUTTON_LEFT, SkaInputTriggerType_PRESSED);
            return 0;
        }
        case WM_LBUTTONUP: {
            ska_input_register_input_event3(SkaInputSourceType_MOUSE, SkaInputKey_MOUSE_BUTTON_LEFT, SkaInputTriggerType_RELEASED);
            return 0;
        }
        case WM_RBUTTONDOWN: {
            ska_input_register_input_event3(SkaInputSourceType_MOUSE, SkaInputKey_MOUSE_BUTTON_RIGHT, SkaInputTriggerType_PRESSED);
            return 0;
        }
        case WM_RBUTTONUP: {
            ska_input_register_input_event3(SkaInputSourceType_MOUSE, SkaInputKey_MOUSE_BUTTON_RIGHT, SkaInputTriggerType_RELEASED);
            return 0;
        }
        case WM_MBUTTONDOWN: {
            ska_input_register_input_event3(SkaInputSourceType_MOUSE, SkaInputKey_MOUSE_BUTTON_MIDDLE, SkaInputTriggerType_PRESSED);
            return 0;
        }
        case WM_MBUTTONUP: {
            ska_input_register_input_event3(SkaInputSourceType_MOUSE, SkaInputKey_MOUSE_BUTTON_MIDDLE, SkaInputTriggerType_RELEASED);
            return 0;
        }
        case WM_MOUSEMOVE: {
            const int x_pos = ((int)(short)LOWORD(l_param));
            const int y_pos = ((int)(short)HIWORD(l_param));
            const SkaVector2i new_position = { x_pos, y_pos };
            ska_input_register_mouse_move_event(new_position);
            return 0;
        }
#endif // SKA_INPUT
        default:
            return DefWindowProc(hwnd, u_msg, w_param, l_param);
    }
}

#endif // #if defined(PLATFORM_WINDOWS)
