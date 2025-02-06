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
#include "templates/template.h"
#include <dbghelp.h>
#include <handleUIInteractions.h>
#pragma comment(lib, "dbghelp.lib")



void cleanup(
		SDLManager* sdl_manager,
		RaylibManager* raylibManager,
		InteractionHandler* interactionHandler,
		CommandProcessor& commandProcessor,
		std::thread& commandThread,
		ImageRescaler* rescaler,
		AVFrame* frame,
		std::unique_ptr<ShouldRenderHandler>& shouldRenderHandler,
		RootNode* rootNode
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
	
//	interactionHandler.stop();
	delete interactionHandler;
	
	delete rootNode;
	
	logger(LogLevel::DEBUG, "END of exit sequence");
}

void initRescaler(RaylibManager* renderManager, AVCodecContext* videoCodecContext, int titleHeight) {
    int width = RaylibGetRenderWidth(), height = RaylibGetRenderHeight();
    logger(LogLevel::DEBUG, "Width of render : " + std::to_string(width));
    logger(LogLevel::DEBUG, "Height of render : " + std::to_string(height));
    WindowSize windowSize{width, height};
    renderManager->resizeWindowFileLoaded(videoCodecContext, windowSize);
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// DEBUG
	bool aborted = false;
	bool renderHandled = false;
	bool renderRequired = false;
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
	
	PlayerThreadHandler* playerHandler = nullptr;
	AVCodecContext* codecContext = nullptr;
	std::unique_ptr<ShouldRenderHandler> renderHandler = nullptr;
	SDL_Event event;
	int titleBarHeight = 31;
	int uiHeight = 113;
	
	InitialParams initialParams = InitialParams{
		100,
		200,
		900,
		505 + titleBarHeight + uiHeight,
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
	char* templateName = "../templates/userInterface.xml";
	pugi::xml_document doc = loadTemplate(templateName);
	
	BasicLayout layout;
	std::shared_ptr<EventQueue> eventQueue = std::make_shared<EventQueue>();
	EventBatch eventBatch;
	InteractionHandler* interactionHandler = new InteractionHandler(eventQueue);
	StyleManager styleManager;
	createStyles(styleManager);
	NodeCreatorWalker walker(styleManager);
	
	doc.traverse(walker);
	RootNode* rootNode = walker.rootNode;
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
		initialParams.xPos,
		initialParams.yPos,
		initialParams.width,
		initialParams.height,
		titleBarHeight,
		uiHeight,
		layout,
		*rescaler,
		"JAGF - Just-Another-Good-FFmpegPlayer"
	);
//	RaylibSetTargetFPS(1000);
	raylibManager->render();
	interactionHandler->acquireRenderableNodes(layout.renderableNodes);
//	interactionHandler.start();	
	
	/*
	* MAIN EVENT HANDLER
	*/
    CommandProcessor commandProcessor(isRunning, sdl_manager->audioDevice, socket_params.port);
    std::thread commandThread(&CommandProcessor::listeningLoop, &commandProcessor);
//    commandProcessor.handleLoad(filePath);


	/*
	* BUTTONS EVENT HANDLERS
	*/
	prepareInteractions(*raylibManager, rootNode, eventQueue, codecContext);
	
	
	//!RaylibWindowShouldClose()
	while (!aborted) {
		renderHandled = false;
		renderRequired = false;
		if (commandProcessor.activeHandlerId != currentPlayerId) {
			logger(LogLevel::DEBUG, "currentPlayer changed");
			currentPlayerId = commandProcessor.activeHandlerId;
			playerHandler = commandProcessor.getPlayerHandlerAt(currentPlayerId);
			codecContext = playerHandler->formatHandler->getVideoCodecContext();
			initRescaler(
				raylibManager,
				codecContext,
				titleBarHeight
			);
			renderHandler = std::make_unique<ShouldRenderHandler>(playerHandler->videoFrameQueue);
		}
		
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
									interactionHandler->consumeEvents();
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
			interactionHandler->consumeEvents();
		}
		
//		logger(LogLevel::DEBUG, "FRAAAAAAAAAME");
//		logger(LogLevel::DEBUG, "Size of event Queue : " + std::to_string(eventQueue.get()->getSize()));
		
		eventQueue->populateBatch(eventBatch);
		for (UIEvent& event : eventBatch.getEvents()) {
			if (event.targetNode == rootNode) {
				if (event.payload.type == EventType::Close) {
					aborted  = true;
				}
			}
			else {
				if (event.payload.type == EventType::MouseMove) {
//					logger(LogLevel::DEBUG, "MouseMove sent");
				}
				event.targetNode->handleEvent(event.payload);
				renderRequired = true;
			}
		}
		
		if (!renderHandled && renderRequired) {
        	raylibManager->render();
		}
		
//		logger(LogLevel::DEBUG, "Size of event Queue : " + std::to_string(eventQueue.get()->getSize()));
		eventBatch.clear();
        
        std::this_thread::sleep_for(std::chrono::microseconds(20));
    }
	
	cleanup(
		sdl_manager,
		raylibManager,
		interactionHandler,
		commandProcessor,
		commandThread,
		rescaler,
		frame,
		renderHandler, // not needed, but here to remind it should be passed by reference
		rootNode
	);
	   
    return 0;
}
