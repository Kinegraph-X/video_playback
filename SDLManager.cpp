#include "player_headers.h"
#include "SDLManager.h"


SDLManager::SDLManager() {}
SDLManager::~SDLManager() {
	reset();
}

// Initialize SDL (video, audio, etc.)
bool SDLManager::initialize() {
	int ret = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_EVENTS);
	if (ret < 0) {
		logger(LogLevel::ERR, std::string("SDL failed to start : ") + std::string(SDL_GetError()));
	}
	else {
		logger(LogLevel::INFO, std::string("SDL successfully started"));
	}
	return true;
}

// Start method: sets up SDL_Window, SDL_Renderer, and SDL_Texture
bool SDLManager::start(int xpos, int ypos, int width, int height, char title[]) {
	this->window = SDL_CreateWindow(
		title,
		xpos,
		ypos,
		width,
		height,
		SDL_WINDOW_RESIZABLE
	);
	
	if (!window) {
        logger(LogLevel::ERR, "Failed to create SDL window: " + std::string(SDL_GetError()));
        SDL_Quit();
        return false;
    }
	
	this->window_id = SDL_GetWindowID(this->window); 
	this->renderer = SDL_CreateRenderer(this->window, -1, 0);
	if (this->renderer == NULL){
		return false;
	}
	this-> texture = SDL_CreateTexture(
		this->renderer,
		SDL_PIXELFORMAT_IYUV,
        SDL_TEXTUREACCESS_STREAMING,
        width,
        height
	);
	
	audioDevice = new AudioDevice();
	audioDevice->startAudioDevice();
	
	return true;
}

// Render method: handles drawing video frames
void SDLManager::render(const uint8_t* y_plane, int y_pitch,
            const uint8_t* u_plane, int u_pitch,
            const uint8_t* v_plane, int v_pitch,
            int video_width, int video_height) {

//	SDL_SetRenderDrawColor;
//	SDL_RenderClear;
//	SDL_QueryTexture;
//	SDL_UpdateYUVTexture;
//	SDL_RenderCopy;
//	SDL_RenderPresent;	
	
}


// Reset method: destroys SDL resources
void SDLManager::reset() {
	cleanUp();
}

// Check if the SDLManager is initialized and ready
bool SDLManager::isReady()  {
	return true;
}

// Internal helper to clean up resources
void SDLManager::cleanUp() {
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}




