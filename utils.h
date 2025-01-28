#pragma once

#include "customizePlatformWindow.h"

std::string GetExecutablePath();
WindowSize getWindowBorders(HWND hwnd);
void subClassWindowProc(HWND hwnd);