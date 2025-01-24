#include "player_headers.h"
#include "utils.h"
#include "SDLManager.h"
#include "SDLAudioDevice.h"
#include "CommandProcessor.h"
#include "SocketServer.h"
#include "ImageRescaler.h"
#include "PlayerThreadHandler.h"
#include "ShouldRenderHandler.h"
#include "DOM/Button.h"


#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")



void cleanup(
		SDLManager* sdl_manager,
		CommandProcessor& commandProcessor,
		std::thread& commandThread,
		ImageRescaler* rescaler,
		AVFrame* frame,
		std::unique_ptr<ShouldRenderHandler>& shouldRenderHandler
	) {
	
    commandProcessor.setAbort();
    if (commandThread.joinable()) {
    	commandThread.join();
	}
	
//	shouldRenderHandler.reset();
	
    if (sdl_manager) {
        sdl_manager->reset();
    }
	
	delete sdl_manager;
	delete rescaler;
	av_frame_free(&frame);
	
	
	
	logger(LogLevel::DEBUG, "END of exit sequence");
}

void initRescaler(SDLManager* sdlManager, AVCodecContext* videoCodecContext, int titleBarHeight) {
    int width = 0, height = 0;
    SDL_GetWindowSize(sdlManager->window, &width, &height);
    height -= titleBarHeight;
    WindowSize windowSize{width, height};
    sdlManager->resizeWindowFileLoaded(videoCodecContext, &windowSize);
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// DEBUG
	bool aborted = false;
	bool resizerOK = false;
	std::string filePath = "\\media\\test_video_02.mp4";
	
	/*
	*  INIT
	*/
	av_log_set_level(AV_LOG_VERBOSE);
	
	AVFrame* frame = av_frame_alloc();
	if (!frame) {
        logger(LogLevel::ERR, "Failed to allocate memory for frame.");
    }
	std::atomic<bool> isRunning(true);
	int currentPlayerId = -1;
	
    AVCodecContext* videoCodecContext = nullptr;
	PlayerThreadHandler* playerHandler = nullptr;
	std::unique_ptr<ShouldRenderHandler> renderHandler = nullptr;
	SDL_Event event;
	
	InitialParams initialParams = InitialParams{
		500,
		200,
		570,
		320,
		av_get_bytes_per_sample(AV_SAMPLE_FMT_S16)
	};
	int titleBarHeight = 31;
	
	Socket_Params socket_params = {
		51312
	};
	
	WindowSize currentWindowPosition {
		initialParams.width,
		initialParams.height,
		initialParams.xPos,
		initialParams.yPos
	};

	/*
	* MAIN PLAYER WINDOW
	*/
	ImageRescaler* rescaler = new ImageRescaler();
	SDLManager* sdl_manager = new SDLManager(titleBarHeight);
	if (!sdl_manager->initialize(rescaler)) {
		logger(LogLevel::ERR, "Exiting after SDL error");
		return -1;
	}
	sdl_manager->start(
		initialParams.xPos,
		initialParams.yPos,
		initialParams.width,
		initialParams.height,
		"JAGF - Just-Another-Good-FFmpegPlayer"
	);
	
	/*
	* MAIN EVENT HANDLER
	*/
    CommandProcessor commandProcessor(isRunning, sdl_manager->audioDevice, socket_params.port);
    std::thread commandThread(&CommandProcessor::listeningLoop, &commandProcessor);
    commandProcessor.handleLoad(filePath);
	
	
	while (isRunning) {
		if (commandProcessor.activeHandlerId != currentPlayerId) {
			logger(LogLevel::DEBUG, "currentPlayer changed");
			currentPlayerId = commandProcessor.activeHandlerId;
			playerHandler = commandProcessor.getPlayerHandlerAt(currentPlayerId);
		    if (playerHandler) {
				videoCodecContext = playerHandler->formatHandler->getVideoCodecContext();
				if (videoCodecContext) { //  && !resizerOK
					initRescaler(sdl_manager, videoCodecContext, titleBarHeight);
					renderHandler = std::make_unique<ShouldRenderHandler>(playerHandler->videoFrameQueue);
					resizerOK = true;
				}
			}
		}
		
	    while (SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT : 
					isRunning.store(false, std::memory_order_release);
					logger(LogLevel::DEBUG, "Received SDL_Quit event");
					break;
				case SDL_USEREVENT : 
					switch (static_cast<PlayerEvent::Type>(event.user.code)) {
						case PlayerEvent::SHOULD_RENDER :
							if (!aborted) {
								if (!(renderHandler->handleRenderEvent(event.user.data1, frame))) {
                                    logger(LogLevel::ERR, "Failed to handle SHOULD_RENDER event.");
                                    aborted = true;
                                }
								else {
									sdl_manager->updateTextureFromFrame(frame);
									av_frame_unref(frame);
								}
							}
							break;		
					}
					break;
				case SDL_WINDOWEVENT :
    				if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
						if (playerHandler && playerHandler->isLoaded) {
							WindowSize windowSize {event.window.data1, event.window.data2};
							sdl_manager->resizeWindowFileLoaded(videoCodecContext, &windowSize);
						}
					}
					else if (event.window.event == SDL_WINDOWEVENT_MOVED) {
			            int offsetX = event.window.data1 - currentWindowPosition.xPos;
			            int offsetY = event.window.data2 - currentWindowPosition.yPos;
//			            uiApplication->updateUIWindowPosition(offsetX, offsetY);
			            
			            currentWindowPosition.xPos = event.window.data1;
			            currentWindowPosition.yPos = event.window.data2;
			        }
					break;
				default:
			      break;
			};
	    }
	    SDL_Delay(16); // ~60 FPS cap
	}
	
	cleanup(
		sdl_manager,
		commandProcessor,
		commandThread,
		rescaler,
		frame,
		renderHandler // not needed, but here to remind it should be passed by reference
	);
	   
    return 0;
}



