#include "PlayerThreadHandler.h"

PlayerThreadHandler::PlayerThreadHandler(AudioDevice* audioDevice) : audioDevice(audioDevice) {
		formatHandler = new AVFormatHandler();
		
		mainThreadHandler = new MainThreadHandler(
			videoPacketQueue,
		    audioPacketQueue,
		    videoFrameQueue,
		    audioFrameQueue,
		    *formatHandler,
		    audioDevice,
		    mediaState,
		    mainThreadOptions
		);
	}

PlayerThreadHandler::~PlayerThreadHandler() {
	cleanup();
}

bool PlayerThreadHandler::loadMedia(const std::string& filePath) {
	bool ret = false;
	
    std::string basePath = GetExecutablePath();
	std::string fullPath = basePath + filePath;
	logger(LogLevel::DEBUG,  "Player thread abs path : " + std::string(fullPath));
	
	const char* filePath_c = fullPath.c_str();
	
	if (formatHandler->openFile(filePath_c) == true) {
		ret = true;
		logger(LogLevel::DEBUG, std::string("Player thead continuing on loaded file"));
		mediaState.status = MediaState::LOADED;
	}
	else {
		logger(LogLevel::ERR, std::string("formatHandler didn't returned true"));
		return ret;
	}

    if (audioDevice->setSWRContext(formatHandler) < 0) {
		logger(LogLevel::ERR, std::string("audioDevice->setSWRContext didn't returned 0"));
		ret = false;
	}
	else  {
		playbackThread = std::thread([this]() {
		    mainThreadHandler->initialize();
		    logger(LogLevel::DEBUG, "mainThreadHandler initialize returned");
		});
		
//		playbackThread.join();
		ret = true;
	}
    
    return ret;
}

void PlayerThreadHandler::play() {
	mainThreadHandler->play();
}

void PlayerThreadHandler::pause() {
    mainThreadHandler->pause();
}

void PlayerThreadHandler::stop() {
    mainThreadHandler->stop();
}

void PlayerThreadHandler::seek(double position) {
    mainThreadHandler->seek(position);
}

void PlayerThreadHandler::playLooped() {
    
}

bool PlayerThreadHandler::checkMinDuration() {
    if (formatHandler->duration < mainThreadOptions.minVideoFrameQueueSize * formatHandler->videoFrameDuration) {
		return false;
	}
	return true;
}

void PlayerThreadHandler::cleanup() {
	logger(LogLevel::DEBUG, "PlayerThreadHandler::cleanup started");
	if (mainThreadHandler) {
        mainThreadHandler->setAbort(true);
    }
	logger(LogLevel::DEBUG, "PlayerThreadHandler::cleanup mainThreadHandler abort set");
	if (playbackThread.joinable()) {
        playbackThread.join();
    }
    logger(LogLevel::DEBUG, "PlayerThreadHandler::cleanup playbackThread ended");
    
	delete formatHandler;
	formatHandler = nullptr;
	delete mainThreadHandler;
	mainThreadHandler = nullptr;
	
	logger(LogLevel::DEBUG, "PlayerThreadHandler::cleanup complete");
}

void PlayerThreadHandler::reset() {
    cleanup();
}