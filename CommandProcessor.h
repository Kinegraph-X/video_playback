#pragma once

#include <unordered_map>
#include <thread>
#include <atomic>
#include <string>
#include "player_headers.h"
#include "SocketServer.h"
#include "PlayerThreadHandler.h"
#include "SDLAudioDevice.h"
#include "FrameQueue.h"
#include "SDLManager.h"
#include "ImageRescaler.h"

class CommandProcessor {
public:
	CommandProcessor(std::atomic<bool>& runningFlag, SocketServer& socketServer, AudioDevice* audioDevice);
    ~CommandProcessor();
	void handleLoad(const std::string& filePath);
    void listeningLoop();
    
    PlayerThreadHandler* getPlayerHandlerAt(int position);
    
    void setAbort();
    void abort();
    
    std::unordered_map<int, PlayerThreadHandler*> playerHandlers;
    
    int activeHandlerId = -1;
    bool errorState = false;
    std::atomic<bool>& isRunning;

private:
    void handlePlay();
    void handlePause();
    void handleStop();
    void cleanUpOldHandler(int id);
	
	std::mutex mutex;
    
    AudioDevice* audioDevice;
    SocketServer& socketServer;

    std::unordered_map<int, std::thread> playerThreads;
    
    int nextHandlerId = 0;
    bool aborted = false;
};

