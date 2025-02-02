#include "player_headers.h"
#include "utils.h"  // IWYU pragma: export
#include "SDLManager.h"
#include "SDLAudioDevice.h" // IWYU pragma: export
#include "CommandProcessor.h"
#include "SocketServer.h" // IWYU pragma: export
#include "ImageRescaler.h"
#include "PlayerThreadHandler.h"
#include "ShouldRenderHandler.h"
#include "DOM/EventQueue.h" // IWYU pragma: export
#include "DOM/InteractionHandler.h" // IWYU pragma: export
#include "RaylibManager.h" // IWYU pragma: export
#include "DOM/Button.h" // IWYU pragma: export
#include "templates/styleDefinitions.h"
#include "templates/testTemplate.h"


#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")



void cleanup(
		SDLManager* sdl_manager,
		RaylibManager* raylibManager,
		CommandProcessor& commandProcessor,
		std::thread& commandThread,
		ImageRescaler* rescaler,
		AVFrame* frame,
		std::unique_ptr<ShouldRenderHandler>& shouldRenderHandler
	) {
	logger(LogLevel::DEBUG, "START of exit sequence");
	
    commandProcessor.setAbort();
    if (commandThread.joinable()) {
    	commandThread.join();
	}
	
	logger(LogLevel::DEBUG, "EXIT sequence commandThread stopped");
//	shouldRenderHandler.reset();

	raylibManager->cleanup();
	delete raylibManager;
	
	logger(LogLevel::DEBUG, "EXIT sequence raylibManager cleaned");
	
    if (sdl_manager) {
        sdl_manager->reset();
    }
	
	delete sdl_manager;
	delete rescaler;
	av_frame_free(&frame);
	
	logger(LogLevel::DEBUG, "END of exit sequence");
}

void initRescaler(RaylibManager* renderManager, AVCodecContext* videoCodecContext, int titleHeight) {
    int width = RaylibGetRenderWidth(), height = RaylibGetRenderHeight();
//    SDL_GetWindowSize(sdlManager->window, &width, &height);
//    height -= titleHeight;
    logger(LogLevel::DEBUG, "Width of render : " + std::to_string(width));
    logger(LogLevel::DEBUG, "Height of render : " + std::to_string(height));
    WindowSize windowSize{width, height};
    renderManager->resizeWindowFileLoaded(videoCodecContext, &windowSize);
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// DEBUG
	bool aborted = false;
//	bool resizerOK = false;
	bool renderHandled = false;
	std::string filePath = "\\media\\test_video_02.mp4";
	
	/*
	*  INIT (GLOBAL VARIABLES DEFINITIONS)
	*/
	av_log_set_level(AV_LOG_VERBOSE);
	
	AVFrame* frame = av_frame_alloc();
	if (!frame) {
        logger(LogLevel::ERR, "Failed to allocate memory for frame.");
    }
	std::atomic<bool> isRunning(true);
	int currentPlayerId = -1;
	
//    AVCodecContext* videoCodecContext = nullptr;
	PlayerThreadHandler* playerHandler = nullptr;
	std::unique_ptr<ShouldRenderHandler> renderHandler = nullptr;
	SDL_Event event;
	int titleBarHeight = 31;
	int uiHeight = 80;
	
	InitialParams initialParams = InitialParams{
		500,
		200,
		570,
		320 + titleBarHeight + uiHeight,
		av_get_bytes_per_sample(AV_SAMPLE_FMT_S16)
	};
	
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
	* BOOTSTRAPPING
	*/
	char* templateName = "../templates/testTemplate.xml";
	pugi::xml_document doc = loadTemplate(templateName);
	
	BasicLayout layout;
	EventQueue eventQueue;
	InteractionHandler interactionHandler(eventQueue);
	StyleManager styleManager;
	createStyles(styleManager);
	NodeCreatorWalker walker(styleManager);
	
	doc.traverse(walker);
	RootNode* rootNode = walker.rootNode;
//	for (auto* child : walker.rootNode->getChildren()) {
//		logger(LogLevel::DEBUG, "ID of children is :  " + child->id);
//		logger(LogLevel::DEBUG, "random computedStyle prop name : " + child->computedStyle.position.name);
//	}
//	for (const auto& property : rootNode->computedStyle) {
//		std::visit([](const auto& prop) {
//			logger(LogLevel::DEBUG, "random computedStyle prop name : " + prop.get().name);
//		}, property);
//	}
	layout.makeLayout(rootNode, currentWindowPosition);

	/*
	* MAIN PLAYER WINDOW
	*/
	ImageRescaler* rescaler = new ImageRescaler();
	SDLManager* sdl_manager = new SDLManager(titleBarHeight);
	if (!sdl_manager->initialize()) {
		logger(LogLevel::ERR, "Exiting after SDL error");
		return -1;
	}
	
	RaylibManager* raylibManager = new RaylibManager(
		initialParams.width,
		initialParams.height,
		titleBarHeight,
		uiHeight,
		layout,
		*rescaler,
		"JAGF - Just-Another-Good-FFmpegPlayer"
	);
	raylibManager->render();
	
	/*
	* MAIN EVENT HANDLER
	*/
    CommandProcessor commandProcessor(isRunning, sdl_manager->audioDevice, socket_params.port);
    std::thread commandThread(&CommandProcessor::listeningLoop, &commandProcessor);
    commandProcessor.handleLoad(filePath);
	
	
//	while (isRunning) {
//		if (commandProcessor.activeHandlerId != currentPlayerId) {
//			logger(LogLevel::DEBUG, "currentPlayer changed");
//			currentPlayerId = commandProcessor.activeHandlerId;
//			playerHandler = commandProcessor.getPlayerHandlerAt(currentPlayerId);
//		    if (playerHandler) {
//				videoCodecContext = playerHandler->formatHandler->getVideoCodecContext();
//				if (videoCodecContext) { //  && !resizerOK
//					initRescaler(sdl_manager, videoCodecContext, titleBarHeight);
//					renderHandler = std::make_unique<ShouldRenderHandler>(playerHandler->videoFrameQueue);
//					resizerOK = true;
//				}
//			}
//		}
//		
//	    while (SDL_PollEvent(&event)) {
//			switch(event.type) {
//				case SDL_QUIT : 
//					isRunning.store(false, std::memory_order_release);
//					logger(LogLevel::DEBUG, "Received SDL_Quit event");
//					break;
//				case SDL_USEREVENT : 
//					switch (static_cast<PlayerEvent::Type>(event.user.code)) {
//						case PlayerEvent::SHOULD_RENDER :
//							if (!aborted) {
//								if (!(renderHandler->handleRenderEvent(event.user.data1, frame))) {
//                                    logger(LogLevel::ERR, "Failed to handle SHOULD_RENDER event.");
//                                    aborted = true;
//                                }
//								else {
////									sdl_manager->updateTextureFromFrame(frame);
//									av_frame_unref(frame);
//								}
//							}
//							break;		
//					}
//					break;
//				case SDL_WINDOWEVENT :
//					if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
//						if (playerHandler && playerHandler->isLoaded) {
//							WindowSize windowSize {event.window.data1, event.window.data2};
//							sdl_manager->resizeWindowFileLoaded(videoCodecContext, &windowSize);
//						}
//					}
//					else if (event.window.event == SDL_WINDOWEVENT_MOVED) {
//			            int offsetX = event.window.data1 - currentWindowPosition.xPos;
//			            int offsetY = event.window.data2 - currentWindowPosition.yPos;
////			            uiApplication->updateUIWindowPosition(offsetX, offsetY);
//			            
//			            currentWindowPosition.xPos = event.window.data1;
//			            currentWindowPosition.yPos = event.window.data2;
//			        }
//					break;
//				default:
//			      break;
//			};
//	    }
//	    SDL_Delay(16); // ~60 FPS cap
//	}
	
//	RaylibSetTargetFPS(120);
	while (!RaylibWindowShouldClose()) {
		renderHandled = false;
		if (commandProcessor.activeHandlerId != currentPlayerId) {
			logger(LogLevel::DEBUG, "currentPlayer changed");
			currentPlayerId = commandProcessor.activeHandlerId;
			playerHandler = commandProcessor.getPlayerHandlerAt(currentPlayerId);
			initRescaler(
				raylibManager,
				playerHandler->formatHandler->getVideoCodecContext(),
				titleBarHeight
			);
			renderHandler = std::make_unique<ShouldRenderHandler>(playerHandler->videoFrameQueue);
		}
		
//        MSG msg;
//        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
//            TranslateMessage(&msg);
//            DispatchMessage(&msg);
//            if (msg.message == WM_CLOSE) {
//				PostQuitMessage(0);
////				RaylibCloseWindow();
//				break;
//			}
//        }
        
        while (SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_USEREVENT : 
					switch (static_cast<PlayerEvent::Type>(event.user.code)) {
						case PlayerEvent::SHOULD_RENDER :
							logger(LogLevel::DEBUG, "RECEIVED SHOULD_RENDER event");
							if (!aborted) {
								if (!(renderHandler->handleRenderEvent(event.user.data1, frame))) {
                                    logger(LogLevel::ERR, "Failed to handle SHOULD_RENDER event.");
                                    aborted = true;
                                }
								else {
									raylibManager->renderFrame(frame);
									renderHandled = true;
									av_frame_unref(frame);
								}
							}
							break;		
					}
					break;
				default:
			      break;
			}
		}
		
		if (!renderHandled) {
			RaylibPollInputEvents();
		}
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
	
	cleanup(
		sdl_manager,
		raylibManager,
		commandProcessor,
		commandThread,
		rescaler,
		frame,
		renderHandler // not needed, but here to remind it should be passed by reference
	);
	   
    return 0;
}



