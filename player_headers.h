#pragma once

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

//#include "RayLib/include/raylib.h"

#define WIN32_LEAN_AND_MEAN
//#define NOGDICAPMASKS
//#define NOVIRTUALKEYCODES
//#define NOWINMESSAGES
//#define NOWINSTYLES
//#define NOSYSMETRICS
//#define NOMENUS
//#define NOICONS
//#define NOKEYSTATES
//#define NOMINMAX
//#define NOSYSCOMMANDS
//#define NORASTEROPS
//#define NOSHOWWINDOW
//#define OEMRESOURCE
//#define NOATOM
//#define NOCLIPBOARD
//#define NOCOLOR
//#define NOCTLMGR
//#define NODRAWTEXT
//#define NOGDI
//#define NOUSER
//#define NOMB
//#define NOMEMMGR
//#define NOMETAFILE
//#define NOMSG
//#define NOOPENFILE
//#define NOSCROLL
//#define NOSERVICE
//#define NOSOUND
//#define NOTEXTMETRIC
//#define NOWH
//#define NOCOMM
//#define NOKANJI
//#define NOHELP
//#define NOPROFILER
//#define NODEFERWINDOWPOS
//#define NOMCX

#include <windows.h>
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

#include <gl/gl.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_syswm.h>


#pragma comment (lib, "SDL2_libs/SDL2main.lib")
#pragma comment (lib, "SDL2_libs/SDL2.lib")

#pragma comment (lib, "RayLib/lib/raylib.lib")

#pragma comment (lib, "User32.lib")
#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "dwmapi.lib")

#define ORIGINAL_WNDPROC_PROP "originalWndProc"

#include "pugiXML/pugixml.hpp"
#include "logger.h"
#include "constants.h"