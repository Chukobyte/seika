#include "win.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    zo_win_init(hInstance, hPrevInstance, lpCmdLine, nCmdShow);

    zo_win_create_window("Test Window", 300, 100, 800, 600);

    while (zo_win_is_active()) {
        zo_win_update_window();
    }

    zo_win_destroy_window();

    return 0;
}

// Include necessary headers

// #include <windows.h>
//
// // Define the name of the window class
// const char CLASS_NAME[] = "Sample Window Class";
//
// // Forward declaration of the Window Procedure
// LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//
// // Entry point for a Windows application
// int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
//     // Step 1: Register the Window Class
//
//     WNDCLASS wc = {0};
//
//     wc.lpfnWndProc   = WindowProc;        // Pointer to the window procedure
//     wc.hInstance     = hInstance;         // Handle to the application instance
//     wc.lpszClassName = CLASS_NAME;        // Name of the window class
//     wc.hCursor       = LoadCursor(NULL, IDC_ARROW); // Default cursor
//     wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);  // Background color
//
//     if (!RegisterClass(&wc)) {
//         MessageBox(NULL, "Window Registration Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
//         return 0;
//     }
//
//     // Step 2: Create the Window
//
//     HWND hwnd = CreateWindowEx(
//         0,                              // Optional window styles
//         CLASS_NAME,                     // Window class
//         "My C11 Win32 Window",          // Window title
//         WS_OVERLAPPEDWINDOW,            // Window style
//
//         // Size and position
//         CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
//
//         NULL,       // Parent window
//         NULL,       // Menu
//         hInstance,  // Instance handle
//         NULL        // Additional application data
//     );
//
//     if (hwnd == NULL) {
//         MessageBox(NULL, "Window Creation Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
//         return 0;
//     }
//
//     // Step 3: Show the Window
//
//     ShowWindow(hwnd, nCmdShow);
//
//     // Step 4: Run the Message Loop
//
//     MSG msg = {0};
//     while (GetMessage(&msg, NULL, 0, 0) > 0) {
//         TranslateMessage(&msg);
//         DispatchMessage(&msg);
//     }
//
//     return (int) msg.wParam;
// }
//
// // Definition of the Window Procedure
// LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
//     switch (uMsg) {
//         case WM_DESTROY:
//             PostQuitMessage(0);
//             return 0;
//
//         // Handle other messages here (e.g., WM_PAINT, WM_SIZE)
//
//         default:
//             return DefWindowProc(hwnd, uMsg, wParam, lParam);
//     }
// }
