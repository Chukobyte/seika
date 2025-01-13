#include "win.h"

typedef struct ZoWin32Data {
    HINSTANCE hInstance;
    int nCmdShow;
} ZoWin32Data;

// Define the name of the window class
static const char CLASS_NAME[] = "Sample Window Class";

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static ZoWin32Data win32Data = {0};
static bool isWindowActive = false;

bool zo_win_init(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
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
    win32Data = (ZoWin32Data){ hInstance, nCmdShow };
    return true;
}

bool zo_win_create_window(const char* title, int posX, int posY, int width, int height) {
    // Step 2: Create the Window

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles
        CLASS_NAME,                     // Window class
        title,          // Window title
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        posX, posY, width, height,

        NULL,       // Parent window
        NULL,       // Menu
        win32Data.hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    ShowWindow(hwnd, win32Data.nCmdShow);
    isWindowActive = true;
    return true;
}

bool zo_win_is_active() { return isWindowActive; }

void zo_win_destroy_window() {}

void zo_win_update_window() {
    MSG msg = {0};
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY: {
            isWindowActive = false;
            PostQuitMessage(0);
            return 0;
        }

        // Handle other messages here (e.g., WM_PAINT, WM_SIZE)

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}
