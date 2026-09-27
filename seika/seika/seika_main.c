#if SKA_USE_MAIN

#include "seika_main.h"

#include "defines.h"

int ska_user_main(int argc, char *argv[]);

#if defined(PLATFORM_WINDOWS)

#include "seika/window/window.h"

int WINAPI WinMain(HINSTANCE h_instance, HINSTANCE h_prev_instance, LPSTR lp_cmd_line, int n_cmd_show) {
    ska_win_init(h_instance, h_prev_instance, lp_cmd_line, n_cmd_show);
    const int argc = __argc;
    char** argv = __argv;
    return ska_user_main(argc, argv);
}
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
#include "seika/window/window.h"

int main(int argc, char** argv) {
    ska_win_init();
    return ska_user_main(argc, argv);
}
#else
int main(int argv, char** args) {
    return ska_user_main(argv, args);
}
#endif // #if defined(PLATFORM_WINDOWS)

#endif // #if SKA_USE_MAIN
