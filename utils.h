#pragma once

#include "player_headers.h"
#include "customizePlatformWindow.h"

std::string GetExecutablePath();
WindowSize getWindowBorders(HWND hwnd);
void subClassWindowProc(HWND hwnd);