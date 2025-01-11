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
	
	logger(LogLevel::INFO, std::string("thread function called"));
	
	threadData->demuxThreadHandler->demuxPackets(
        threadData->options->maxVideoPacketQueueSize,
        threadData->options->maxAudioPacketQueueSize
    );
    return 0;
};







MainThreadHandler::MainThreadHandler(
	PacketQueue& videoPacketQueue,
    PacketQueue& audioPacketQueue,
    FrameQueue& videoFrameQueue,
    FrameQueue& audioFrameQueue,
    AVFormatHandler& formatHandler,
    
    MediaState& mediaState,
    const MainThreadOptions& opts
) : 
	videoPacketQueue(videoPacketQueue),
    audioPacketQueue(audioPacketQueue),
    videoFrameQueue(videoFrameQueue),
    audioFrameQueue(audioFrameQueue),
    formatHandler(formatHandler),
    
    mediaState(mediaState),
    options(opts) {}
MainThreadHandler::~MainThreadHandler() {
	cleanUp();
}

void MainThreadHandler::initialize() {
	currentTimestamp = 0.;
	elapsedPlayingTime = 0.;
	frameDuration = formatHandler.videoFrameDuration;

    demuxThreadHandler = new DemuxThreadHandler(videoPacketQueue, audioPacketQueue, formatHandler);
    threadData = {demuxThreadHandler, &options};

	if (!demuxThreadHandler) {
	    logger(LogLevel::ERR, "demuxThreadHandler creation failed");
	}

    // Set up the threads (using SDL_CreateThread, etc.)
    createThreads();
    mainLoop();
}

void MainThreadHandler::mainLoop() {
	while (true) {
		updateTiming();
		
		if (videoPacketQueue.getSize() < options.minVideoPacketQueueSize) {
			demuxThreadHandler->wakeUpThread();
		}
		
		if (mediaState.status == MediaState::LOADED
				&& currentTimestamp - lastFrameTimestamp >= frameDuration) {
			
			logger(LogLevel::INFO, std::string("elapsed time is ") + LogUtils::toString(elapsedPlayingTime));
			
			lastFrameTimestamp = currentTimestamp;
		}
		
		if (elapsedPlayingTime > 2.) {
			demuxThreadHandler->stopThread();
			break;
		}
	}
}


double MainThreadHandler::updateTiming() {
//	logger(LogLevel::INFO, std::string("currentTimestamp ") + LogUtils::toString(currentTimestamp));
    if (mediaState.status == MediaState::LOADED) {
        double now = (double)SDL_GetPerformanceCounter() / SDL_GetPerformanceFrequency();
        if (currentTimestamp == 0.) {
            currentTimestamp = now;  // Initialize on first call
            lastFrameTimestamp = now;
        }
		elapsedPlayingTime += now - currentTimestamp;
		currentTimestamp = now;
    }
    return 0.;
}


// Clean-up resources if needed
void MainThreadHandler::cleanUp() {
//	TerminateThread(demuxThreadID, 0);
    delete demuxThreadHandler;
}


// Create threads for video and audio
void MainThreadHandler::createThreads() {
    
    SDL_Thread* demuxThread = SDL_CreateThread(
        MainThreadHandlerHelpers::demuxThreadFunction,
        "DemuxThread",
        &threadData
    );
    
    if (!demuxThread) {
	    logger(LogLevel::ERR, "Failed to create demux thread");
	    return;
	}
//	demuxThreadID = SDL_GetThreadID(demuxThread);
}









