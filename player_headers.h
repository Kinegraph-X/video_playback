#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#include <gl/gl.h>

#define SDL_VIDEO_RENDER_D3D
#define SDL_VIDEO_RENDER_OGL

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_syswm.h>

#include "raylib_headers.h" // IWYU pragma: export

#include <cstdio> // IWYU pragma: export
#include <cstdlib> // IWYU pragma: export
#include <iostream> // IWYU pragma: export
#include <string> // IWYU pragma: export
#include <atomic> // IWYU pragma: export
#include <mutex> // IWYU pragma: export
#include <functional> // IWYU pragma: export
#include <map> // IWYU pragma: export
#include<unordered_map> // IWYU pragma: export
#include <unordered_set> // IWYU pragma: export
#include <queue> // IWYU pragma: export


//#include "DOM\EventListener.h"

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
    #include <libswresample/swresample.h>
}
#pragma comment (lib, "ffmpeg_libs/avformat.lib")
#pragma comment (lib, "ffmpeg_libs/avcodec.lib")
#pragma comment (lib, "ffmpeg_libs/avutil.lib")
#pragma comment (lib, "ffmpeg_libs/swscale.lib")
#pragma comment (lib, "ffmpeg_libs/swresample.lib")



#pragma comment (lib, "SDL2_libs/SDL2main.lib")
#pragma comment (lib, "SDL2_libs/SDL2.lib")

#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "dwmapi.lib")

#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "shell32.lib")
//#pragma comment(lib, "msvcrt.lib")
#pragma comment(lib, "RayLib/lib/raylib.lib")




#define ORIGINAL_WNDPROC_PROP "originalWndProc"

//#include "pugiXML/pugixml.hpp"
#include "logger.h"  // IWYU pragma: export
#include "constants.h" // IWYU pragma: export