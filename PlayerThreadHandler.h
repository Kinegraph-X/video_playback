#pragma once

#include "player_headers.h"
#include "utils.h"
#include "AVFormatHandler.h"
#include "SDLAudioDevice.h"
#include "PacketQueue.h"
#include "FrameQueue.h"
#include "MediaState.h"
#include "MainThreadHandler.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>

class PlayerThreadHandler {
public:
    PlayerThreadHandler(AudioDevice* audioDevice);
    ~PlayerThreadHandler();

    bool loadMedia(const std::string& filePath);
    void play();
    void pause();
    void stop();
    void playLooped();
    void seek(double position);
    bool checkMinDuration();
    void abort();
    void reset();
	
	AVFormatHandler* formatHandler = nullptr;    
    MainThreadHandler* mainThreadHandler = nullptr;
    
    FrameQueue videoFrameQueue;
    bool isLoaded = false;
    std::thread playbackThread;

private:

    AudioDevice* audioDevice;

    PacketQueue videoPacketQueue;
    PacketQueue audioPacketQueue;
    FrameQueue audioFrameQueue;
    
    MediaState mediaState;
    MainThreadOptions mainThreadOptions;
    
    void cleanup();
};

