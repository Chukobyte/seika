#if SKA_USE_MAIN

#include "seika_main.h"

#include "defines.h"

int ska_user_main(int argc, char *argv[]);

#if defined(PLATFORM_WINDOWS)

#include "seika/window/window.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    ska_win_init(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
    const int argc = __argc;
    char** argv = __argv;
    return ska_user_main(argc, argv);
}
#else
int main(int argv, char** args) {
    return ska_user_main(argv, args);
}
#endif // #if defined(PLATFORM_WINDOWS)

#endif // #if SKA_USE_MAIN
