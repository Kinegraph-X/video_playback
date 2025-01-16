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
//	logger(LogLevel::ERR, "PlayerThreadHandler destructor called");
	delete formatHandler;
	delete mainThreadHandler;
    stop();
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
		});// this will loop infinitely
//		logger(LogLevel::ERR, std::string("mainThreadHandler->initialize didn't returned true"));
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

void PlayerThreadHandler::seek(double timestamp) {
    mainThreadHandler->seek(timestamp);
}

void PlayerThreadHandler::playLooped() {
    
}

bool PlayerThreadHandler::checkMinDuration() {
    if (formatHandler->duration < mainThreadOptions.minVideoFrameQueueSize * formatHandler->videoFrameDuration) {
		return false;
	}
	return true;
}

void PlayerThreadHandler::abort() {
	logger(LogLevel::DEBUG, "PlayerThreadHandler::abort called");
	
	mainThreadHandler->cleanUp();
	
	if (playbackThread.joinable()) {
        playbackThread.join();
    }
	delete mainThreadHandler;
}


