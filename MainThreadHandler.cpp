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
	cleanup();
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
	
	double minQueueSize = std::ceil(formatHandler.audioSampleRate 
									/ formatHandler.videoFrameRate); // upper closest than exactly one video frame
	options.minAudioDeviceQueueSize = static_cast<size_t>(std::ceil(minQueueSize));
	
	// Adapt in case the audio driver is refusing the buffer size we requested
	// (prioritize powers of 2 as most soundcards impose these increments)
	while (options.minAudioDeviceQueueSize < audioDevice->availableSpec.samples) {
		options.minAudioDeviceQueueSize = static_cast<size_t>(std::ceil(minQueueSize)) * multiplicator;
		multiplicator = multiplicator << 1;
	}
	options.minAudioDeviceQueueSize = options.minAudioDeviceQueueSize * av_get_bytes_per_sample(formatHandler.sampleFormat);
	
	logger(LogLevel::INFO, "Defined minAudioDeviceQueueSize to : " + LogUtils::toString(options.minAudioDeviceQueueSize));
	logger(LogLevel::INFO, "minAudioDeviceQueueSize duration : " + LogUtils::toString((double)options.minAudioDeviceQueueSize / (formatHandler.audioSampleRate * av_get_bytes_per_sample(formatHandler.sampleFormat))));
}



//		std::this_thread::sleep_for(std::chrono::microseconds(100));

void MainThreadHandler::mainLoop() {
	while (!isAborted()) {
		
		{
			std::lock_guard<std::mutex> lock(mutex);
			
			if (seekRequested) {
				manageStatus();
				continue;
			}
			
			if (stopRequested) {
				playRequested = false;
				pauseRequested = false;
				mediaState.status = MediaState::PAUSED;
				continue;
			}
			
			if (mediaState.status == MediaState::PLAYING
					&& currentTimestamp - lastFrameTimestamp >= frameDuration) {
						
				if (enqueuedAudioDuration < elapsedPlayingTime + frameDuration) {
					manageAudioQueue();
				}
				
//				if (elapsedPlayingTime >= formatHandler.duration - frameDuration) {
//			        // We're near the end, switch to a transitional state
//			        mediaState.status = MediaState::ENDING;
//			        continue;
//			    }
			
				if (pauseRequested) {
					SDL_PauseAudioDevice(audioDevice->deviceID, 1);
					playRequested = false;
					mediaState.status = MediaState::PAUSED;
					continue;
				}
				
				SDL_PauseAudioDevice(audioDevice->deviceID, 0);
				
				// Guard against race conditions
				if (!videoFrameQueue.isEmpty()) {
					SDL_PushEvent(&newEvent(PlayerEvent::SHOULD_RENDER));
				}
//				logger(LogLevel::INFO, std::string("elapsed time is ") + LogUtils::toString(elapsedPlayingTime));
				
				lastFrameTimestamp += frameDuration;
			
//				if (elapsedPlayingTime > 5.) {
//					logger(LogLevel::DEBUG, std::string("Playback stopped on debug condition") + LogUtils::toString(mediaState.status));
//					demuxThreadHandler->stopThread();
//					decodeThreadHandler->stopThread();
//					playRequested = false;
//					pauseRequested = true;
//					mediaState.status = MediaState::PAUSED;
//					break;
//				}
			}
			
			if (elapsedPlayingTime >= formatHandler.duration
					&& videoFrameQueue.getSize() == 0
					&& audioFrameQueue.getSize() == 0) {
				logger(LogLevel::INFO, std::string("Playback stopped on duration exceeded : ") + LogUtils::toString(formatHandler.duration));
				
				SDL_PauseAudioDevice(audioDevice->deviceID, 1);
				demuxThreadHandler->stopThread();
				decodeThreadHandler->stopThread();
				playRequested = false;
				pauseRequested = true;
				mediaState.status = MediaState::ENDED;
				
				break;
			}
			
		

		updateTiming();
		manageStatus();
		
		} // We unlock the mutex here
		
		std::this_thread::sleep_for(std::chrono::microseconds(10));
	}
	logger(LogLevel::DEBUG, std::string("MainThreadHandler : Playback exited its infinite loop"));
}

void MainThreadHandler::manageStatus() {
	if (mediaState.status == MediaState::UNDEFINED
//		|| mediaState.status == MediaState::ENDING 
		|| mediaState.status == MediaState::ENDED) {
		return;
	}
	
	if (!demuxThreadHandler->exhausted) {
		if (seekRequested) {
			if (videoPacketQueue.getSize() < options.seekMinVideoPacketQueueSize
					|| audioPacketQueue.getSize() < options.seekMinAudioPacketQueueSize) {
				if (mediaState.status != MediaState::BUFFERING) {
					logStatus(MediaState::BUFFERING, "Demuxer");
				}
				mediaState.status = MediaState::BUFFERING;
				demuxThreadHandler->wakeUpThread();
				return;
			}
		}
		else {
			if (videoPacketQueue.getSize() < options.minVideoPacketQueueSize
					|| audioPacketQueue.getSize() < options.minAudioPacketQueueSize) {
				if (mediaState.status != MediaState::BUFFERING) {
					logStatus(MediaState::BUFFERING, "Demuxer");
				}
				mediaState.status = MediaState::BUFFERING;
				demuxThreadHandler->wakeUpThread();
				return;
			}
		}
	}
	else if (demuxThreadHandler->exhausted) {
//		logger(LogLevel::DEBUG, "The Demuxer has exhausted the file.");
	}
	
	if (seekRequested) {
		if (!decodeThreadHandler->exhausted
				&&(videoFrameQueue.getSize() < options.seekMinVideoFrameQueueSize
					|| audioFrameQueue.getSize() < options.seekMinAudioFrameQueueSize)
			) {
			if (mediaState.status != MediaState::BUFFERING)
				logStatus(MediaState::BUFFERING, "Decoder");
			mediaState.status = MediaState::BUFFERING;
			decodeThreadHandler->wakeUpThread();
			return;
		}
		else {
	//		logger(LogLevel::DEBUG, "tEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEst for ENOUGHDATATOPLAY");
			if (mediaState.status != MediaState::ENOUGHDATATOPLAY)
				logStatus(MediaState::ENOUGHDATATOPLAY, "Decoder");
			mediaState.status = MediaState::ENOUGHDATATOPLAY;
		}
	}
	else {
		if (!decodeThreadHandler->exhausted
				&& (videoFrameQueue.getSize() < options.minVideoFrameQueueSize
					|| audioFrameQueue.getSize() < options.minAudioFrameQueueSize)
			) {
			if (mediaState.status != MediaState::BUFFERING)
				logStatus(MediaState::BUFFERING, "Decoder");
			mediaState.status = MediaState::BUFFERING;
			decodeThreadHandler->wakeUpThread();
			return;
		}
		else if (playRequested) {
	//		logger(LogLevel::DEBUG, "tEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEst for playbackRequested");
			if (mediaState.status != MediaState::PLAYING && mediaState.status != MediaState::ENDING) {
				logStatus(mediaState.status, "Decoder");
				mediaState.status = MediaState::PLAYING;
			}
		}
		else {
	//		logger(LogLevel::DEBUG, "tEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEst for ENOUGHDATATOPLAY");
			if (mediaState.status != MediaState::ENOUGHDATATOPLAY)
				logStatus(MediaState::ENOUGHDATATOPLAY, "Decoder");
			mediaState.status = MediaState::ENOUGHDATATOPLAY;
		}
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
//    	logger(LogLevel::DEBUG, std::string("Enqueued audio time is ") + LogUtils::toString(enqueuedAudioDuration));
//    	audioDevice->printStatus();
    }
}

SDL_Event MainThreadHandler::newEvent(PlayerEvent::Type type) {
    PlayerEvent* eventData = new PlayerEvent{type, elapsedPlayingTime};
    SDL_Event user_event;
    SDL_zero(user_event);
    user_event.type = SDL_USEREVENT;
    user_event.user.code = static_cast<Sint32>(type);
    user_event.user.data1 = eventData;
    
    return user_event;
}


// Create threads for video and audio
void MainThreadHandler::createThreads() {
	
    demuxThreadData = {demuxThreadHandler, &options};
    decodeThreadData = {decodeThreadHandler, &options};
    
	demuxThread = std::thread(MainThreadHandlerHelpers::demuxThreadFunction, &demuxThreadData);
	decodeThread = std::thread(MainThreadHandlerHelpers::decodeThreadFunction, &decodeThreadData);
}

void MainThreadHandler::play() {
	while (mediaState.status != MediaState::ENOUGHDATATOPLAY) {
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
	std::lock_guard<std::mutex> lock(mutex);
	playRequested = true;
	pauseRequested = false;
	stopRequested = false;
	logger(LogLevel::INFO, std::string("play requested : ") + LogUtils::toString(playRequested));
}

void MainThreadHandler::pause() {
	std::lock_guard<std::mutex> lock(mutex);
	pauseRequested = true;
	stopRequested = false;
}

/**
 * @param position expressed in seconds
*/
void MainThreadHandler::seek(double position) {
	double now = (double)SDL_GetPerformanceCounter() / SDL_GetPerformanceFrequency();
	stop();
	
	{
		seekRequested = true;
		std::lock_guard<std::mutex> lock(mutex);
		
		avcodec_flush_buffers(formatHandler.getVideoCodecContext());
		avcodec_flush_buffers(formatHandler.getAudioCodecContext());
		
		int frameCount = static_cast<int>(std::round(position * formatHandler.videoFrameRate));
		avformat_seek_file(formatHandler.getFormatContext(), 0, AV_NOPTS_VALUE, position, AV_NOPTS_VALUE, AVSEEK_FLAG_FRAME);
		elapsedPlayingTime = position;
		enqueuedAudioDuration = position;
		currentTimestamp = 0.;
	}
	while (videoFrameQueue.getSize() == 0) {
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
	seekRequested = false;
	pauseRequested = true;
	stopRequested = false;
	mediaState.status = MediaState::PAUSED;
	logger(LogLevel::DEBUG, "time needed for seeking : " + LogUtils::toString((double)SDL_GetPerformanceCounter() / SDL_GetPerformanceFrequency() - now));
}

void MainThreadHandler::stop() {
	std::lock_guard<std::mutex> lock(mutex);
	playRequested = false;
	stopRequested = true;
	pauseRequested = false;
	demuxThreadHandler->stopThread();
	decodeThreadHandler->stopThread();
	decodeThreadHandler->exhausted = false;
	demuxThreadHandler->exhausted = false;
	resetQueues();
}

void MainThreadHandler::resetQueues() {
	videoPacketQueue.flush();
	audioPacketQueue.flush();
	videoFrameQueue.flush();
	audioPacketQueue.flush();
}

void MainThreadHandler::setAbort(bool value) {
	abort.store(value, std::memory_order_release);
	
	// Give mainLoop a chance to exit
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
	
//	std::lock_guard<std::mutex> lock(mutex);
	std::unique_lock<std::mutex> lock(mutex);

	logger(LogLevel::DEBUG, "MainThreadHandler aborting.");
    
    // First set abort flags on queues and handlers
    videoPacketQueue.setAbort(value);
    audioPacketQueue.setAbort(value);
    logger(LogLevel::DEBUG, "MainThreadHandler abort set on queues.");
    
    demuxThreadHandler->setAbort(value);
    decodeThreadHandler->setAbort(value);
    
    logger(LogLevel::DEBUG, "MainThreadHandler abort set on threads.");
    // Then stop threads (which should now exit quickly due to abort flags)
    demuxThreadHandler->stopThread();
    decodeThreadHandler->stopThread();
    resetQueues();
}

bool MainThreadHandler::isAborted() const { return abort.load(std::memory_order_acquire); }

// Clean-up resources if needed
void MainThreadHandler::cleanup() {
    logger(LogLevel::DEBUG, "MainThreadHandler::cleanup started.");
    setAbort(true);
    stop();

    if (demuxThreadHandler) {
        if (demuxThread.joinable()) {
            demuxThread.join();
        }
        delete demuxThreadHandler;
        demuxThreadHandler = nullptr;
    }

    if (decodeThreadHandler) {
        if (decodeThread.joinable()) {
            decodeThread.join();
        }
        delete decodeThreadHandler;
        decodeThreadHandler = nullptr;
    }
    
    logger(LogLevel::DEBUG, "MainThreadHandler::cleanup completed.");
}

void MainThreadHandler::reset() {
    cleanup();  // Call cleanup to reset resources
}

void MainThreadHandler::logStatus(MediaState::State status, std::string origin) {
	if (status == MediaState::BUFFERING) {
		logger(LogLevel::DEBUG, std::string("player status changed to buffering...") + std::string(" due to ") + origin + std::string(" state management"));
	}
	else {
		logger(LogLevel::DEBUG, std::string("player status changed to ") + LogUtils::toString(status) + std::string(" due to ") + origin + std::string(" state management"));
	}
}



