#include "window.h"

#if defined(PLATFORM_WINDOWS)

#include <stdio.h>
#include <io.h> // For _open_osfhandle
#include <fcntl.h> // For _O_TEXT
#include <wingdi.h>
#include "seika/logger.h"
#include "seika/rendering/renderer.h"
#include "seika/input/input_win32.h"

static SkaWindow mainWindow = {0};

typedef struct ZoWin32Data {
    HINSTANCE hInstance;
    HWND hwnd; // Should attach to window data if we ever support multiple windows
    HDC hdc;
    int nCmdShow;
} ZoWin32Data;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Define the name of the window class
static const char CLASS_NAME[] = "Zo Window Class";

static ZoWin32Data win32Data = {0};
static bool isOpenGLInitialized = false;

// OPENGL
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0X2092
#define WGL_CONTEXT_FLAGS_ARB 0X2094
#define WGL_CONTEXT_COREPROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
typedef const char* (WINAPI* PFNWGLGETEXTENSIONSSTRINGEXTPROC)(void);
typedef BOOL (WINAPI* PFNWGLSWAPINTERVALEXTPROC)(int);
typedef int (WINAPI* PFNWGLGETSWAPINTERVALEXTPROC) (void);
typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC hDC, HGLRC hShareContext, const int* attribList);

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
    win32Data.hdc = hdc;
    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixelFormat, &pfd);

    HGLRC tempRC = wglCreateContext(hdc);
    wglMakeCurrent(hdc, tempRC);
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    const int attribList[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_FLAGS_ARB, 0,
        WGL_CONTEXT_PROFILE_MASK_ARB,
        WGL_CONTEXT_COREPROFILE_BIT_ARB, 0,
        0 // Terminator
    };

    HGLRC hglrc = wglCreateContextAttribsARB(hdc, 0, attribList);
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(tempRC);
    wglMakeCurrent(hdc, hglrc);

    if (!gladLoadGL()) {
        ska_logger_error("Could not initialize GLAD!");
    }
    else {
        ska_logger_debug("OpenGL Version %d", GLVersion.major);
    }

    PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
    bool swapControlSupported = strstr(_wglGetExtensionsStringEXT(), "WGL_EXT_swap_control") != 0;
    //https://www.khronos.org/opengl/wiki/Swap_Interval
    int vsynch = 0;

    if (swapControlSupported) {
        PFNWGLSWAPINTERVALEXTPROC wglSwapInternalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
        PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
        if (wglSwapInternalEXT(1)) {
            ska_logger_debug("VSynch enabled");
        }
        else {
            ska_logger_error("Could not enable VSynch");
        }
    }
    else {
        ska_logger_error("WGL_EXT_swap_control not supported");
    }
    isOpenGLInitialized = true;
}

static void enable_console() {
    if (AttachConsole(GetCurrentProcessId()) == 0) { // Try to attach to parent console
        AllocConsole(); // If no parent console, allocate a new one
    }
    FILE* pFile;
    freopen_s(&pFile, "CONOUT$", "w", stdout);
}

bool ska_win_init(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Register the Window Class
    WNDCLASS wc = {0};
    wc.lpfnWndProc   = WindowProc;        // Pointer to the window procedure
    wc.hInstance     = hInstance;         // Handle to the application instance
    wc.lpszClassName = CLASS_NAME;        // Name of the window class
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW); // Default cursor
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);  // Background color

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }
    win32Data = (ZoWin32Data){ .hInstance = hInstance, .hdc = NULL, .nCmdShow = nCmdShow };
    enable_console();

    return true;
}

SkaWindow* ska_win_create_window(SkaCreateWindowProps props) {
    const char* title = props.title;
    const int32 posX = props.posX;
    const int32 posY = props.posY;
    const int32 width = (int32)props.width;
    const int32 height = (int32)props.height;
    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles
        CLASS_NAME,                     // Window class
        title,                          // Window title
        WS_OVERLAPPEDWINDOW,            // Window style
        // Size and position
        posX, posY, width, height,
        NULL,                           // Parent window
        NULL,                           // Menu
        win32Data.hInstance,            // Instance handle
        NULL                            // Additional application data
    );

    if (hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    win32Data.hwnd = hwnd;
    ShowWindow(hwnd, win32Data.nCmdShow);

    opengl_init(hwnd);

    mainWindow = (SkaWindow){ .title = title, .width = width, .height = height, .isActive = true };
    ska_win_update_size(&mainWindow, width, height);
    return &mainWindow;
}

SkaWindow* ska_win_get_primary_window() {
    return mainWindow.isActive ? &mainWindow : NULL;
}

bool ska_win_is_active(const SkaWindow* window) { return window->isActive; }

void ska_win_destroy_window(SkaWindow* window) {
    window->isActive = false;
    isOpenGLInitialized = false;
}

void ska_win_process_events(SkaWindow* window) {
    MSG msg = {0};
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void ska_win_swap_window(SkaWindow* window) {
    SwapBuffers(win32Data.hdc);
}

void ska_win_update_size(SkaWindow* window, int32 width, int32 height) {
    window->width = width;
    window->height = height;
    ska_renderer_update_window_size(width, height);
}

void ska_win_enable_hscroll_bar(SkaWindow* window, bool enable) {
    LONG style = GetWindowLong(win32Data.hwnd, GWL_STYLE);
    if (enable) {
        style |= WS_HSCROLL;
    } else {
        style &= ~WS_HSCROLL;
    }
    SetWindowLong(win32Data.hwnd, GWL_STYLE, style);
    SetWindowPos(win32Data.hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

void ska_win_enable_vscroll_bar(SkaWindow* window, bool enable) {
    LONG style = GetWindowLong(win32Data.hwnd, GWL_STYLE);
    if (enable) {
        style |= WS_VSCROLL;
    } else {
        style &= ~WS_VSCROLL;
    }
    SetWindowLong(win32Data.hwnd, GWL_STYLE, style);
    SetWindowPos(win32Data.hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY: {
            mainWindow.isActive = false;
            PostQuitMessage(0);
            return 0;
        }
        case WM_SIZE: {
            if (isOpenGLInitialized) {
                const int32 width = LOWORD(lParam);
                const int32 height = HIWORD(lParam);
                ska_win_update_size(&mainWindow, width, height); // TODO: Using mainWindow here maybe fix later?
                mainWindow.width = width;
                mainWindow.height = height;
            }
            return 0;
        }
        case WM_KEYDOWN: {
            const uint32 vkCode = (uint32)wParam;
            const SkaInputKey translatedKey = ska_input_win32_vkcode_to_keycode(vkCode);
            ska_input_register_input_event3(SkaInputSourceType_KEYBOARD, translatedKey, SkaInputTriggerType_PRESSED);
            return 0;
        }
        case WM_KEYUP: {
            const uint32 vkCode = (uint32)wParam;
            const SkaInputKey translatedKey = ska_input_win32_vkcode_to_keycode(vkCode);
            ska_input_register_input_event3(SkaInputSourceType_KEYBOARD, translatedKey, SkaInputTriggerType_RELEASED);
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
            const int xPos = ((int)(short)LOWORD(lParam));
            const int yPos = ((int)(short)HIWORD(lParam));
            const SkaVector2i newPosition = { xPos, yPos };
            ska_input_register_mouse_move_event(newPosition);
            return 0;
        }
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

#endif // #if defined(PLATFORM_WINDOWS)
