#include "player_headers.h"
#include "utils.h"
#include "constants.h"
#include "SDLManager.h"
#include "SDLAudioDevice.h"
#include "CommandProcessor.h"
#include "SocketServer.h"
#include "ImageRescaler.h"
//#include "AVFormatHandler.h"
//#include "PacketQueue.h"
//#include "FrameQueue.h"
//#include "MediaState.h"
//#include "MainThreadHandler.h"


Initial_Params initial_params = {
	500,
	200,
	570,
	320,
	av_get_bytes_per_sample(AV_SAMPLE_FMT_S16)
};

Socket_Params socket_params = {
	51312
};



//void log_callback(void *ptr, int level, const char *fmt, va_list vargs) {
//	char buffer[1024];
//    snprintf(buffer, sizeof(buffer), fmt, vargs);
//    logger(LogLevel::DEBUG, std::string(buffer));
//};

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	av_log_set_level(AV_LOG_VERBOSE);
//	av_log_set_callback(log_callback);

	// Define a global state for running the main loop
	std::atomic<bool> isRunning(true);

//	SetDllDirectory(GetExecutablePath().c_str());
//	logger(LogLevel::INFO, "Application Started");

//	MediaState mediaState;

	ImageRescaler rescaler;
	
	SDLManager sdl_manager;
	if (!sdl_manager.initialize()) {
		logger(LogLevel::ERR, "Exiting after SDL error");
		return -1;
	}
	sdl_manager.start(
		initial_params.xpos,
		initial_params.ypos,
		initial_params.width,
		initial_params.height,
		"Video Player"
	);
	
	std::string filePath = "\\media\\test_video.mp4";
	
	// Initialize TCP server
    SocketServer socketServer(socket_params.port);
    if (!socketServer.start()) {
        logger(LogLevel::ERR, "Failed to start TCP server");
        sdl_manager.cleanUp();
        SDL_Quit();
        return -1;
    }
    
    CommandProcessor commandProcessor(isRunning, socketServer, sdl_manager.audioDevice, rescaler);
    std::thread commandThread(&CommandProcessor::listeningLoop, &commandProcessor);
    commandProcessor.handleLoad(filePath);
	
//	std::string basePath = GetExecutablePath();
//	std::string fullPath = basePath + "\\media\\test_video.mp4";
//	
//	const char* filePath = fullPath.c_str();
//	AVFormatHandler* formatHandler = &AVFormatHandler();
//	if (formatHandler->openFile(filePath) == true) {
//		logger(LogLevel::INFO, std::string("MediaState::LOADED"));
//		mediaState.status = MediaState::LOADED;
//	};
//	
//	audioDevice->setSWRContext(formatHandler);
//	
//	PacketQueue videoPacketQueue;
//    PacketQueue audioPacketQueue;
//    FrameQueue videoFrameQueue;
//    FrameQueue audioFrameQueue;
//    MainThreadOptions mainThreadOptions;
//    
//	MainThreadHandler* mainThreadHandler = new MainThreadHandler(
//		videoPacketQueue,
//	    audioPacketQueue,
//	    videoFrameQueue,
//	    audioFrameQueue,
//	    *formatHandler,
//	    *audioDevice,
//	    mediaState,
//	    mainThreadOptions
//	);
//	mainThreadHandler->initialize();
	
	
	SDL_Event event;
	
	while (isRunning) {
	    while (SDL_PollEvent(&event)) {
	        if (event.type == SDL_QUIT) {
	            isRunning = false; // Exit loop on window close
//	            mainThreadHandler->setAbort(true); 
	        }
	    }
	    SDL_Delay(16); // ~60 FPS cap
	}
	
	// Cleanup
    isRunning = false;
	
	rescaler.cleanUp();
    socketServer.stop();
    sdl_manager.cleanUp();
    commandProcessor.abort();
    commandThread.join();
    
    SDL_Quit();

    return 0;
}
