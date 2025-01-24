#include "CommandProcessor.h"




CommandProcessor::CommandProcessor(std::atomic<bool>& runningFlag, AudioDevice* audioDevice, unsigned short socketPort)
    : isRunning(runningFlag), audioDevice(audioDevice), socketServer(socketPort) {
		
    if (!socketServer.start()) {
        logger(LogLevel::ERR, "Failed to start TCP server");
    }
}

CommandProcessor::~CommandProcessor() {
    setAbort();
}

void CommandProcessor::listeningLoop() {
    while (isRunning.load(std::memory_order_acquire)) {
//		if (activeHandlerId != -1) {
//			logger(LogLevel::DEBUG, "STATE of playerThread : " + LogUtils::toString(playerThreads[activeHandlerId].joinable()));
//			logger(LogLevel::DEBUG, "STATE of playbackThread : " + LogUtils::toString(playerHandlers[activeHandlerId]->playbackThread.joinable()));
//		}
        std::string command = socketServer.receiveCommand();
        if (command.empty()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}
		
        if (command.rfind("LOAD:", 0)) {
            std::string filePath = command.substr(5);
            handleLoad(filePath);
        } else if (command == "PLAY") {
            handlePlay();
        } else if (command == "PAUSE") {
            handlePause();
        } else if (command == "STOP") {
            handleStop();
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    logger(LogLevel::DEBUG, std::string("CommandProcessor::listeningLoop exited"));
}

void CommandProcessor::handleLoad(const std::string& filePath) {
	errorState = false;
	if (!audioDevice) {
		logger(LogLevel::ERR, "audioDevice is NULL : " + std::string(filePath));
		return;
	}
	
    PlayerThreadHandler* playerHandler = new PlayerThreadHandler(audioDevice);
    int currentHandlerId = nextHandlerId++;
    playerHandlers[currentHandlerId] = playerHandler;

    // Start a thread to load media and wait for it to be ready
    playerThreads[currentHandlerId] = std::thread([this, playerHandler, filePath, currentHandlerId]() {
		logger(LogLevel::INFO, "Player handler thread running... " + std::string(filePath));
        if (playerHandler->loadMedia(filePath)) {
            logger(LogLevel::DEBUG, "Command processor continuing after successfull file loading: " + std::string(filePath));
            
            if (!playerHandler->checkMinDuration()) {
				errorState = true;
				cleanUpOldHandler(currentHandlerId);
				return;
			}
			
			logger(LogLevel::DEBUG, "CommandProcessor errorState: " + LogUtils::toString(errorState));
			
			playerHandler->isLoaded = true;

            if (activeHandlerId >= 0 && activeHandlerId != currentHandlerId) {
                cleanUpOldHandler(activeHandlerId);
            }
            
            // Play the new media
            playerHandler->play();
            
            activeHandlerId = currentHandlerId;
            
//            std::this_thread::sleep_for(std::chrono::seconds(8));
//            playerHandler->seek(8);

        } else {
            logger(LogLevel::ERR, "Failed to load file: " + std::string(filePath));
            delete playerHandler;
            playerHandlers.erase(currentHandlerId);
        }
    });
}

void CommandProcessor::handlePlay() {
    if (activeHandlerId >= 0 && playerHandlers.count(activeHandlerId)) {
        playerHandlers[activeHandlerId]->play();
    }
}

void CommandProcessor::handlePause() {
    if (activeHandlerId >= 0 && playerHandlers.count(activeHandlerId)) {
        playerHandlers[activeHandlerId]->pause();
    }
}

void CommandProcessor::handleStop() {
    if (activeHandlerId >= 0 && playerHandlers.count(activeHandlerId)) {
        cleanUpOldHandler(activeHandlerId);
        activeHandlerId = -1;
    }
}

PlayerThreadHandler* CommandProcessor::getPlayerHandlerAt(int position) {

	if (playerHandlers.count(position) > 0) {
		return playerHandlers.at(position);
	}
	return nullptr;
}

void CommandProcessor::cleanUpOldHandler(int id) {
    if (playerThreads.count(id)) {
        if (playerHandlers.count(id)) {
            playerHandlers[id]->stop();
        }

        if (playerThreads[id].joinable()) {
            playerThreads[id].join();
        }

        delete playerHandlers[id];
        playerHandlers.erase(id);
        playerThreads.erase(id);
    }
}

void CommandProcessor::abort() {
	logger(LogLevel::DEBUG, "CommandProcessor::abort called");
	
	for (auto& [id, thread] : playerThreads) {
        if (thread.joinable()) thread.join();
    }
    for (auto& [id, handler] : playerHandlers) {
        delete handler;
    }
    socketServer.reset();
    logger(LogLevel::DEBUG, "CommandProcessor abort sequence ended");
}

void CommandProcessor::setAbort() {
	// abort() will be called on exiting the main scope, but we first need to call it manually,
	// join the threads before cleaning the rest.
	// So there's a mechanism preenting multiple calls.
	logger(LogLevel::DEBUG, "CommandProcessor::setAbort called, aborted is : " + LogUtils::toString(aborted));
	if (!aborted) {
		logger(LogLevel::DEBUG, "CommandProcessor should abort");
		aborted = true;
		abort();
	}
}
