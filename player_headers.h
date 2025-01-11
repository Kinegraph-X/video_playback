#pragma once

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}
# pragma comment (lib, "ffmpeg_libs/avformat.lib")
# pragma comment (lib, "ffmpeg_libs/avcodec.lib")
# pragma comment (lib, "ffmpeg_libs/avutil.lib")
# pragma comment (lib, "ffmpeg_libs/swscale.lib")
#include <SDL2/SDL.h>

#include "logger.h"

//# pragma comment (lib, "SDL2_libs/SDL2main.lib")
# pragma comment (lib, "SDL2_libs/SDL2.lib")

# pragma comment (lib, "User32.lib")


