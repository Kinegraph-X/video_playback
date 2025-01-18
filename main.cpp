#include "player_headers.h"
#include "utils.h"
#include "constants.h"
#include "SDLManager.h"
#include "SDLAudioDevice.h"
#include "CommandProcessor.h"
#include "SocketServer.h"
#include "ImageRescaler.h"
#include "PlayerThreadHandler.h"
#include "ShouldRenderHandler.h"



void initRescaler(SDLManager* sdlManager, AVCodecContext* videoCodecContext) {
    int width = 0, height = 0;
    SDL_GetWindowSize(sdlManager->window, &width, &height);
    WindowSize windowSize{width, height};
    sdlManager->resizeWindowFileLoaded(videoCodecContext, &windowSize);
}



int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	av_log_set_level(AV_LOG_VERBOSE);
	
	InitialParams* initialParams = new InitialParams{
		500,
		200,
		570,
		320,
		av_get_bytes_per_sample(AV_SAMPLE_FMT_S16)
	};
	
	Socket_Params socket_params = {
		51312
	};

	std::atomic<bool> isRunning(true);

//	SetDllDirectory(GetExecutablePath().c_str());

	ImageRescaler* rescaler = new ImageRescaler();
	
	SDLManager* sdl_manager = new SDLManager();
	if (!sdl_manager->initialize(rescaler)) {
		logger(LogLevel::ERR, "Exiting after SDL error");
		return -1;
	}
	sdl_manager->start(
		initialParams->xpos,
		initialParams->ypos,
		initialParams->width,
		initialParams->height,
		"Video Player"
	);
	
	std::string filePath = "\\media\\test_video.mp4";
	
	// Initialize TCP server
    SocketServer socketServer(socket_params.port);
    if (!socketServer.start()) {
        logger(LogLevel::ERR, "Failed to start TCP server");
        sdl_manager->cleanUp();
        SDL_Quit();
        return -1;
    }
    
    CommandProcessor commandProcessor(isRunning, socketServer, sdl_manager->audioDevice);
    std::thread commandThread(&CommandProcessor::listeningLoop, &commandProcessor);
    commandProcessor.handleLoad(filePath);
	
	// DEBUG
	bool aborted = false;
	
	bool resizerOK = false;
	int currentPlayerId = -1;
	
	AVFrame* frame = av_frame_alloc();
	if (!frame) {
        logger(LogLevel::ERR, "Failed to allocate memory for frame.");
    }
    
    AVCodecContext* videoCodecContext = nullptr;
	PlayerThreadHandler* playerHandler = nullptr;
	std::unique_ptr<ShouldRenderHandler> renderHandler = nullptr;
	
	SDL_Event event;
	
	while (isRunning) {
		if (commandProcessor.activeHandlerId != currentPlayerId) {
			currentPlayerId = commandProcessor.activeHandlerId;
			playerHandler = commandProcessor.getPlayerHandlerAt(currentPlayerId);
		    if (playerHandler) {
				videoCodecContext = playerHandler->formatHandler->getVideoCodecContext();
				if (videoCodecContext && !resizerOK) {
					initRescaler(sdl_manager, videoCodecContext);
					renderHandler = std::make_unique<ShouldRenderHandler>(playerHandler->videoFrameQueue);
					resizerOK = true;
				}
			}
		}
		
	    while (SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT : 
					isRunning = false;
					break;
				case SDL_USEREVENT : 
					switch (static_cast<PlayerEvent::Type>(event.user.code)) {
						case PlayerEvent::SHOULD_RENDER :
							if (!aborted && resizerOK) {
								
								if (!(renderHandler->handleRenderEvent(event.user.data1, frame))) {
                                    logger(LogLevel::ERR, "Failed to handle SHOULD_RENDER event.");
                                    aborted = true;
                                }
								else {
									sdl_manager->updateTextureFromFrame(frame);
									av_frame_unref(frame);
								}
								aborted = true;
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
					break;
				default:
//			      logger(LogLevel::ERR, "Unhandled Event!");
			      break;
			};
	    }
	    SDL_Delay(16); // ~60 FPS cap
	}
	
	// Cleanup
	
	av_frame_free(&frame);
    socketServer.reset();
    commandProcessor.abort();
    commandThread.join();
    
    delete rescaler;
	delete sdl_manager;
	    
    SDL_Quit();

    return 0;
}



