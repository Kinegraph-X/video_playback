#include "player_headers.h"
#include "SDLManager.h"


SDLManager::SDLManager(int titleBarHeight) : titleBarHeight(titleBarHeight) {}
SDLManager::~SDLManager() {
	reset();
}

// Initialize SDL (video, audio, etc.)
bool SDLManager::initialize(ImageRescaler* rescaler) {
	int ret = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_EVENTS);
	if (ret < 0) {
		logger(LogLevel::ERR, std::string("SDL failed to start : ") + std::string(SDL_GetError()));
	}
	else {
		logger(LogLevel::INFO, std::string("SDL successfully started"));
	}
	
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	
	this->rescaler = rescaler;
	return true;
}

// Start method: sets up SDL_Window, SDL_Renderer, and SDL_Texture
bool SDLManager::start(int xpos, int ypos, int width, int height, char title[]) {
	windowWidth = width;
	
	this->window = SDL_CreateWindow(
		title,
		xpos,
		ypos,
		width,
		height + this->titleBarHeight,
		SDL_WINDOW_RESIZABLE //| SDL_WINDOW_BORDERLESS // SDL_WINDOW_OPENGL
	);
	
	if (!window) {
        logger(LogLevel::ERR, "Failed to create SDL window: " + std::string(SDL_GetError()));
        SDL_Quit();
        return false;
    }
    this->window_id = SDL_GetWindowID(this->window);
    hwnd = this->getNativeWindowHandle();
//    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    
    // Create renderer
    if (!initRenderer()) {
        cleanup();
        return false;
    }

    // Prepare textures for video and custom title bar
    if (!initTextures(width, height)) {
        cleanup();
        return false;
    }
	
	audioDevice = new AudioDevice();
	audioDevice->startAudioDevice();
	
//	render();
	
	return true;
}

bool SDLManager::initGLContext() {
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        logger(LogLevel::ERR, "Failed to create OpenGL context: " + std::string(SDL_GetError()));
        return false;
    }
    this->window_id = SDL_GetWindowID(this->window);
    return true;
}

bool SDLManager::initRenderer() {
    this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!this->renderer) {
        logger(LogLevel::ERR, "Failed to create renderer: " + std::string(SDL_GetError()));
        return false;
    }
    return true;
}

bool SDLManager::initTextures(int width, int height) {
    // Create texture for video rendering
    this->videoTexture = SDL_CreateTexture(
        this->renderer,
        SDL_PIXELFORMAT_IYUV,
        SDL_TEXTUREACCESS_STREAMING,
        width,
        height
    );
    if (!this->videoTexture) {
        logger(LogLevel::ERR, "Failed to create video texture: " + std::string(SDL_GetError()));
        return false;
    }

    // Create texture for the custom title bar
    this->titleBarTexture = SDL_CreateTexture(
        this->renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        width,
        this->titleBarHeight // Title bar height
    );
    if (!this->titleBarTexture) {
        logger(LogLevel::ERR, "Failed to create title bar texture: " + std::string(SDL_GetError()));
        return false;
    }

    return true;
}

void SDLManager::render() {
    // Clear the screen
    SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 0);
    SDL_RenderClear(this->renderer);

    // Render the title bar
    renderTitleBar();

    // Present everything to the screen
    SDL_RenderPresent(this->renderer);
}

void SDLManager::renderTitleBar() {
//	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Set the renderer target to the title bar texture
    SDL_SetRenderTarget(this->renderer, this->titleBarTexture);

    // Draw the custom title bar background (e.g., a solid color)
    SDL_SetRenderDrawColor(this->renderer, 5, 18, 24, 128); // Dark color
    SDL_RenderClear(this->renderer);

    // Draw title text (if desired)
    // Use SDL_ttf or another library to render text here

    // Reset the render target
    SDL_SetRenderTarget(this->renderer, nullptr);

    // Render the title bar texture to the window
    SDL_Rect titleBarRect = {0, 0, windowWidth, this->titleBarHeight};
    SDL_RenderCopy(this->renderer, this->titleBarTexture, nullptr, &titleBarRect);
}



HWND SDLManager::getNativeWindowHandle() {
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(this->window, &wmInfo);
	HWND nativeWindowHandle = wmInfo.info.win.window;
	return nativeWindowHandle;
}

void SDLManager::resizeWindowFileLoaded(AVCodecContext* codecContext, const WindowSize* initialParams) {
	resetRescaler(codecContext, initialParams);
}

void SDLManager::resetRescaler(AVCodecContext* codecContext, const WindowSize* initialParams) {
	rescaler->initializeSwsContext(codecContext, initialParams);
}

void SDLManager::updateTextureFromFrame(AVFrame* frame) {
    if (!videoTexture || !frame) {
        logger(LogLevel::ERR, "Texture or frame is null.");
        return;
    }
    int result = 0;
    WindowSize windowSize {
		0,
		0
	};
	SDL_GetWindowSize(window, &windowSize.width, &windowSize.height);
	windowSize.height -= this->titleBarHeight;

    // SDL_UpdateYUVTexture requires the frame to be in YUV420P format.
    if (frame->format != AV_PIX_FMT_YUV420P || frame->width != windowSize.width || frame->height != windowSize.height) {
//        logger(LogLevel::DEBUG, "Unsupported pixel format. Will rescale...");
        
		AVFrame* scaledFrame = rescaler->rescaleFrame(frame, windowSize);
		clampLuminance(scaledFrame, 20);
		
		result = SDL_UpdateYUVTexture(
	        videoTexture,
	        nullptr,                       // Update the entire texture.
	        scaledFrame->data[0], scaledFrame->linesize[0], // Y plane.
	        scaledFrame->data[1], scaledFrame->linesize[1], // U plane.
	        scaledFrame->data[2], scaledFrame->linesize[2]  // V plane.
	    );
	    av_frame_free(&scaledFrame);
    }
	else {
		clampLuminance(frame, 20);
	    // Update the texture using the planes of the AVFrame.
	    result = SDL_UpdateYUVTexture(
	        videoTexture,
	        nullptr,                       // Update the entire texture.
	        frame->data[0], frame->linesize[0], // Y plane.
	        frame->data[1], frame->linesize[1], // U plane.
	        frame->data[2], frame->linesize[2]  // V plane.
	    );
    }
    
    if (result != 0) {
        logger(LogLevel::ERR, std::string("Failed to update texture: ") + SDL_GetError());
    }
    
    SDL_RenderClear(renderer);
    
//    renderTitleBar();
    
	SDL_Rect videoDestRect = {
        0,                              // x: Start at the left edge
        this->titleBarHeight,           // y: Start below the title bar
        windowSize.width,               // width: Match window width
        windowSize.height               // height: Match adjusted height
    };

    SDL_RenderCopy(renderer, videoTexture, nullptr, &videoDestRect);
//    SetLayeredWindowAttributes(hwnd, RGB(0,0,0), 255, LWA_COLORKEY); // LWA_ALPHA
    SDL_RenderPresent(renderer);

}

void SDLManager::clampLuminance(AVFrame* frame, uint8_t minLuminance) {
    if (!frame || frame->format != AV_PIX_FMT_YUV420P) {
        return;
    }

    int width = frame->width;
    int height = frame->height;
    uint8_t* yPlane = frame->data[0];
    int yStride = frame->linesize[0];

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (yPlane[y * yStride + x] < minLuminance) {
                yPlane[y * yStride + x] = minLuminance;
            }
        }
    }
}



// Reset method: destroys SDL resources
void SDLManager::reset() {
	cleanup();
}

void SDLManager::cleanup() {
	logger(LogLevel::DEBUG, "SDLManager::cleanup started.");
	if (audioDevice) {
		delete audioDevice;
		audioDevice = nullptr;
	}
	
	if (videoTexture) SDL_DestroyTexture(videoTexture);
    if (titleBarTexture) SDL_DestroyTexture(titleBarTexture);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	logger(LogLevel::DEBUG, "SDLManager::cleanup ended.");
}





