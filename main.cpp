#include "player_headers.h"
#include "utils.h"
#include "constants.h"
#include "SDLManager.h"
#include "AVFormatHandler.h"
#include "PacketQueue.h"
#include "PacketQueue.h"
#include "FrameQueue.h"
#include "FrameQueue.h"
#include "MediaState.h"
#include "MainThreadHandler.h"

Initial_Params initial_params = {
	500,
	200,
	570,
	320,
	av_get_bytes_per_sample(AV_SAMPLE_FMT_S16)
};

void log_callback(void *ptr, int level, const char *fmt, va_list vargs) {
	char buffer[1024];
    snprintf(buffer, sizeof(buffer), fmt, vargs);
    logger(LogLevel::DEBUG, std::string(buffer));
};

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	av_log_set_level(AV_LOG_VERBOSE);
//	av_log_set_callback(log_callback);

//	SetDllDirectory(GetExecutablePath().c_str());
//	logger(LogLevel::INFO, "Application Started");

	MediaState mediaState;
	
	SDLManager sdl_manager;
	sdl_manager.start(
		initial_params.xpos,
		initial_params.ypos,
		initial_params.width,
		initial_params.height,
		"Video Player"
	);
	
	std::string basePath = GetExecutablePath();
	std::string fullPath = basePath + "\\media\\test_video.mp4";

	const char* filePath = fullPath.c_str();
	AVFormatHandler* formatHandler = &AVFormatHandler();
	if (formatHandler->openFile(filePath) == true) {
		logger(LogLevel::INFO, std::string("MediaState::LOADED"));
		mediaState.status = MediaState::LOADED;
	};
	
	PacketQueue videoPacketQueue;
    PacketQueue audioPacketQueue;
    FrameQueue videoFrameQueue;
    FrameQueue audioFrameQueue;
    MainThreadOptions mainThreadOptions;
    
	MainThreadHandler* mainThreadHandler = new MainThreadHandler(
		videoPacketQueue,
	    audioPacketQueue,
	    videoFrameQueue,
	    audioFrameQueue,
	    *formatHandler,
	    mediaState,
	    mainThreadOptions
	);
	mainThreadHandler->initialize();
	
	bool running = true;
	
	SDL_Event event;
	
	while (running) {
	    while (SDL_PollEvent(&event)) {
	        if (event.type == SDL_QUIT) {
	            running = false; // Exit loop on window close
	        }
	    }
	
	    // Your rendering and logic here
	    SDL_Delay(16); // ~60 FPS cap
	}
	
    return 0;
}
