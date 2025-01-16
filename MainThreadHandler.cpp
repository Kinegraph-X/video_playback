#include <MainThreadHandler.h>




int MainThreadHandlerHelpers::demuxThreadFunction(void *data) {
	DemuxThreadData* threadData =  static_cast<DemuxThreadData*>(data);
	
	if (!threadData->options) {
	    logger(LogLevel::ERR, "Invalid thread options");
	    return -1;
	}
	else if (!threadData->demuxThreadHandler) {
	    logger(LogLevel::ERR, "Invalid thread handler");
	    return -1;
	}
	
	logger(LogLevel::DEBUG, std::string("demux thread function called"));
	
	threadData->demuxThreadHandler->demuxPackets(
        threadData->options->maxVideoPacketQueueSize,
        threadData->options->maxAudioPacketQueueSize
    );
    return 0;
};

int MainThreadHandlerHelpers::decodeThreadFunction(void *data) {
	DecodeThreadData* threadData =  static_cast<DecodeThreadData*>(data);
	
	if (!threadData->options) {
	    logger(LogLevel::ERR, "Invalid thread options");
	    return -1;
	}
	else if (!threadData->decodeThreadHandler) {
	    logger(LogLevel::ERR, "Invalid thread handler");
	    return -1;
	}
	
	logger(LogLevel::DEBUG, std::string("decode thread function called"));
	
	threadData->decodeThreadHandler->decodePackets(
        threadData->options->maxVideoFrameQueueSize,
        threadData->options->maxAudioFrameQueueSize
    );
    return 0;
};







MainThreadHandler::MainThreadHandler(
	PacketQueue& videoPacketQueue,
    PacketQueue& audioPacketQueue,
    FrameQueue& videoFrameQueue,
    FrameQueue& audioFrameQueue,
    AVFormatHandler& formatHandler,
    AudioDevice* audioDevice,
    
    MediaState& mediaState,
    MainThreadOptions& opts
) : 
	videoPacketQueue(videoPacketQueue),
    audioPacketQueue(audioPacketQueue),
    videoFrameQueue(videoFrameQueue),
    audioFrameQueue(audioFrameQueue),
    formatHandler(formatHandler),
    audioDevice(audioDevice),
    
    mediaState(mediaState),
    options(opts) {}
MainThreadHandler::~MainThreadHandler() {
	cleanUp();
}

bool MainThreadHandler::initialize() {
	currentTimestamp = 0.;
	elapsedPlayingTime = 0.;
	frameDuration = formatHandler.videoFrameDuration;
	setMinAudioDeviceQueueSize();

//	logger(LogLevel::INFO, std::string("options.minAudioDeviceQueueSize ") + LogUtils::toString(options.minAudioDeviceQueueSize));
//	logger(LogLevel::INFO, std::string("formatHandler.audioChannelLayout->nb_channels ") + LogUtils::toString(formatHandler.audioChannelLayout->nb_channels));
//	logger(LogLevel::INFO, std::string("av_get_bytes_per_sample(formatHandler.sampleFormat) ") + LogUtils::toString(av_get_bytes_per_sample(formatHandler.sampleFormat)));
	

    demuxThreadHandler = new DemuxThreadHandler(videoPacketQueue, audioPacketQueue, formatHandler);
//    demuxThreadData = {demuxThreadHandler, &options};
    decodeThreadHandler = new DecodeThreadHandler(videoPacketQueue, audioPacketQueue, videoFrameQueue, audioFrameQueue, formatHandler);
//    decodeThreadData = {decodeThreadHandler, &options};

	if (!demuxThreadHandler) {
	    logger(LogLevel::ERR, "demuxThreadHandler creation failed");
	}
	if (!decodeThreadHandler) {
	    logger(LogLevel::ERR, "decodeThreadHandler creation failed");
	}

    // Set up the threads (using SDL_CreateThread, etc.)
    createThreads();
    mainLoop();
    
    return true;
}

void MainThreadHandler::setMinAudioDeviceQueueSize() {
	int multiplicator = 2;
	
	int minQueueSize = (formatHandler.audioSampleRate 
									* av_get_bytes_per_sample(formatHandler.sampleFormat)
									/ formatHandler.videoCodecPar->framerate.num); // exactly one video frame
	options.minAudioDeviceQueueSize = minQueueSize;
	
	// Adapt in case the audio driver is refusing the buffer size we requested
	// (prioritize powers of 2 as most soundcards impose these increments)
	while (options.minAudioDeviceQueueSize < audioDevice->availableSpec.samples) {
		options.minAudioDeviceQueueSize = minQueueSize * multiplicator;
		multiplicator = multiplicator << 1;
	}
}



//		std::this_thread::sleep_for(std::chrono::microseconds(100));

void MainThreadHandler::mainLoop() {
	while (!isAborted()) {
		updateTiming();
		manageStatus();
		
		if (mediaState.status == MediaState::PLAYING
				&& currentTimestamp - lastFrameTimestamp >= frameDuration) {
			
			if (enqueuedAudioDuration < currentTimestamp + frameDuration) {
				manageAudioQueue();
			}
			
			if (pauseRequested) {
				SDL_PauseAudioDevice(audioDevice->deviceID, 1);
				playRequested = false;
				mediaState.status = MediaState::PAUSED;
				continue;
			}
			
			SDL_PauseAudioDevice(audioDevice->deviceID, 0);
			
			logger(LogLevel::INFO, std::string("elapsed time is ") + LogUtils::toString(elapsedPlayingTime));
//			logger(LogLevel::INFO, std::string("duration is ") + LogUtils::toString(formatHandler.duration));
			
			lastFrameTimestamp += frameDuration;
		}
		
		if (elapsedPlayingTime > 5.) {
			logger(LogLevel::DEBUG, std::string("Playback stopped on debug condition"));
			demuxThreadHandler->stopThread();
			decodeThreadHandler->stopThread();
			break;
		}
		// Due to multithreading, the loop is started before the file is completely open => ensure we have a duration
		if (formatHandler.duration != 0. && elapsedPlayingTime >= formatHandler.duration) {
			logger(LogLevel::INFO, std::string("Playback stopped on duration exceeded : ") + LogUtils::toString(formatHandler.duration));
			mediaState.status = MediaState::ENDED;
			demuxThreadHandler->stopThread();
			decodeThreadHandler->stopThread();
			break;
		}
		
		std::this_thread::sleep_for(std::chrono::microseconds(10));
	}
	logger(LogLevel::DEBUG, std::string("Playback exited its infinite loop"));
}

void MainThreadHandler::manageStatus() {
	if (mediaState.status == MediaState::UNDEFINED || mediaState.status == MediaState::ENDED) {
		return;
	}
	
	if (!demuxThreadHandler->exhausted
			&& (videoPacketQueue.getSize() < options.minVideoPacketQueueSize
				|| audioPacketQueue.getSize() < options.minAudioPacketQueueSize)) {
		if (mediaState.status != MediaState::BUFFERING) {
			logStatus(MediaState::BUFFERING, "Demuxer");
		}
		mediaState.status = MediaState::BUFFERING;
		demuxThreadHandler->wakeUpThread();
		return;
	}
	else if (demuxThreadHandler->exhausted) {
		logger(LogLevel::DEBUG, "The Demuxer has exhausted the file.");
	}
	
	if ((videoPacketQueue.getSize() >= options.minVideoPacketQueueSize
			&& videoFrameQueue.getSize() < options.minVideoFrameQueueSize)
			|| (audioPacketQueue.getSize() >= options.minAudioPacketQueueSize
				&& audioFrameQueue.getSize() < options.minAudioFrameQueueSize)
		) {
		if (mediaState.status != MediaState::BUFFERING)
			logStatus(MediaState::BUFFERING, "Decoder");
		mediaState.status = MediaState::BUFFERING;
		decodeThreadHandler->wakeUpThread();
		return;
	}
	else if (playRequested) {
//		logger(LogLevel::DEBUG, "tEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEst for playbackRequested");
		if (mediaState.status != MediaState::PLAYING)
			logStatus(mediaState.status, "Decoder");
		mediaState.status = MediaState::PLAYING;
	}
	else {
//		logger(LogLevel::DEBUG, "tEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEst for ENOUGHDATATOPLAY");
		if (mediaState.status != MediaState::ENOUGHDATATOPLAY)
			logStatus(MediaState::ENOUGHDATATOPLAY, "Decoder");
		mediaState.status = MediaState::ENOUGHDATATOPLAY;
	}
}

double MainThreadHandler::updateTiming() {
    if (mediaState.status == MediaState::PLAYING) {
        double now = (double)SDL_GetPerformanceCounter() / SDL_GetPerformanceFrequency();
        if (currentTimestamp == 0.) {
            currentTimestamp = now;
            lastFrameTimestamp = now;
        }
		elapsedPlayingTime += now - currentTimestamp;
		currentTimestamp = now;
    }
    return 0.;
}

void MainThreadHandler::manageAudioQueue() {
	int ret;
	double queuedTime = 0.;
	bool shouldLog = false;
	
	while (audioDevice->getQueuedAudioSize() < options.minAudioDeviceQueueSize
			&& !audioFrameQueue.isEmpty()) {
		
		shouldLog = true;
        AVFrame* audioFrame = av_frame_alloc();
        
        audioFrameQueue.get(audioFrame);
        if (audioFrame) {
		    queuedTime += (double)audioFrame->nb_samples / formatHandler.audioSampleRate;

            if ((ret = audioDevice->queueFrame(audioFrame, &formatHandler)) < 0) {
				logger(LogLevel::ERR, std::string("audio frame queue error ") + std::string(SDL_GetError()));
			}
            av_frame_free(&audioFrame);
        }
    }
    enqueuedAudioDuration += queuedTime;
    if (shouldLog) {
//		logger(LogLevel::INFO, std::string("audio device needs BUFFERING"));
//    	logger(LogLevel::INFO, std::string("Enqueued audio time is ") + LogUtils::toString(enqueuedAudioDuration));
//    	audioDevice->printStatus();
    }
}

void MainThreadHandler::setAbort(bool value) {
    abort = value;
    resetQueues();
    videoPacketQueue.setAbort(value);
    audioPacketQueue.setAbort(value);
    demuxThreadHandler->setAbort(value);
    decodeThreadHandler->setAbort(value);
}

bool MainThreadHandler::isAborted() const { return abort; }

// Clean-up resources if needed
void MainThreadHandler::cleanUp() {
    logger(LogLevel::INFO, "Shutting down threads...");
    setAbort(true);
    stop();

    if (demuxThreadHandler) {
        delete demuxThreadHandler;
        demuxThreadHandler = nullptr;
        if (demuxThread.joinable()) {
            demuxThread.join();
        }
    }

    if (decodeThreadHandler) {
        delete decodeThreadHandler;
        decodeThreadHandler = nullptr;
        if (decodeThread.joinable()) {
            decodeThread.join();
        }
    }
    
    logger(LogLevel::INFO, "Cleanup completed.");
}


// Create threads for video and audio
void MainThreadHandler::createThreads() {
	
    demuxThreadData = {demuxThreadHandler, &options};
    decodeThreadData = {decodeThreadHandler, &options};
    
//    demuxThread = SDL_CreateThread(
//        MainThreadHandlerHelpers::demuxThreadFunction,
//        "DemuxThread",
//        &demuxThreadData
//    );

	demuxThread = std::thread(MainThreadHandlerHelpers::demuxThreadFunction, &demuxThreadData);
    
//    if (!demuxThread) {
//	    logger(LogLevel::ERR, "Failed to create demux thread");
//	    return;
//	}
//	demuxThreadID = SDL_GetThreadID(demuxThread);

//	decodeThread = SDL_CreateThread(
//        MainThreadHandlerHelpers::decodeThreadFunction,
//        "DecodeThread",
//        &decodeThreadData
//    );

	decodeThread = std::thread(MainThreadHandlerHelpers::decodeThreadFunction, &decodeThreadData);
    
//    if (!decodeThread) {
//	    logger(LogLevel::ERR, "Failed to create decode thread");
//	    return;
//	}
}

void MainThreadHandler::resetQueues() {
	videoPacketQueue.flush();
	audioPacketQueue.flush();
	videoFrameQueue.flush();
	audioPacketQueue.flush();
}

void MainThreadHandler::play() {
	playRequested = true;
	pauseRequested = false;
	logger(LogLevel::INFO, std::string("play requested : ") + LogUtils::toString(playRequested));
}

void MainThreadHandler::pause() {
	pauseRequested = true;
}

void MainThreadHandler::stop() {
	stopRequested = true;
	resetQueues();
//	demuxThreadHandler->setAbort(true);
//	decodeThreadHandler->setAbort(true);
}

void MainThreadHandler::seek(double position) {
	seekRequested = true;
	stop();
	resetQueues();
	
	// see how to restart the threads the most elegantly possible
}

void MainThreadHandler::logStatus(MediaState::State status, std::string origin) {
	if (status == MediaState::BUFFERING) {
		logger(LogLevel::DEBUG, std::string("player status changed to buffering...") + std::string(" due to ") + origin + std::string(" state management"));
	}
	else {
		logger(LogLevel::DEBUG, std::string("player status changed to ") + LogUtils::toString(status) + std::string(" due to ") + origin + std::string(" state management"));
	}
}







