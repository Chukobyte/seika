#pragma once

#include <stdbool.h>
#include <windows.h>

bool zo_win_init(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
bool zo_win_create_window(const char* title, int posX, int posY, int width, int height);
void zo_win_destroy_window();
bool zo_win_is_active();
void zo_win_update_window();
