#include "CommandProcessor.h"




CommandProcessor::CommandProcessor(std::atomic<bool>& runningFlag, SocketServer& socketServer, AudioDevice* audioDevice, ImageRescaler& rescaler)
    : isRunning(runningFlag), audioDevice(audioDevice), socketServer(socketServer), rescaler(rescaler) {
//    if (!socketServer.start()) {
//        throw std::runtime_error("Failed to start SocketServer");
//    }
}

CommandProcessor::~CommandProcessor() {
    abort();
}

void CommandProcessor::listeningLoop() {
    while (isRunning) {
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
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void CommandProcessor::handleLoad(const std::string& filePath) {
	errorState = false;
	if (!audioDevice) {
		logger(LogLevel::ERR, "audioDevice is NULL : " + std::string(filePath));
		return;
	}
	
    // Create a new PlayerThreadHandler
    PlayerThreadHandler* playerHandler = new PlayerThreadHandler(audioDevice);
    int currentHandlerId = nextHandlerId++;
    playerHandlers[currentHandlerId] = playerHandler;

    // Start a thread to load media and wait for it to be ready
    // , currentFrameQueue
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

            // Wait until media is ready to play
            while (playerHandler->mainThreadHandler->mediaState.status != MediaState::ENOUGHDATATOPLAY) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            logger(LogLevel::INFO, "Media ready to play: " + std::string(filePath));
            
            rescaler.initializeSwsContext(playerHandler->formatHandler->getVideoCodecContext());
			
//			currentFrameQueue = &playerHandler.videoFrameQueue;

            // If there's an active handler, stop and clean it up
            if (activeHandlerId >= 0 && activeHandlerId != currentHandlerId) {
                cleanUpOldHandler(activeHandlerId);
            }
            
            // Play the new media
            playerHandler->play();

            activeHandlerId = currentHandlerId;

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
	
	for (auto& [id, handler] : playerHandlers) {
		logger(LogLevel::DEBUG, "calling abort for playerHandler id  : " + LogUtils::toString(id));
		handler->abort();
    }
	
	for (auto& [id, thread] : playerThreads) {
        if (thread.joinable()) thread.join();
    }
    for (auto& [id, handler] : playerHandlers) {
        delete handler;
    }
    socketServer.stop();
}
