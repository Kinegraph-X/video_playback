#pragma once

#include "player_headers.h"
#include "SDLAudioDevice.h"
#include "ImageRescaler.h"

class SDLManager {
public:
    SDLManager(int titleBarHeight);
    ~SDLManager();

    bool initialize();

    // sets up SDL_Window, SDL_Renderer, and SDL_Texture
    bool start(int xpos, int ypos, int width, int height, char title[]);
	HWND SDLManager::getNativeWindowHandle();
	
	bool SDLManager::initGLContext();
	bool SDLManager::initRenderer();
	bool SDLManager::initTextures(int width, int height);
	void SDLManager::render();
	void SDLManager::renderTitleBar();
	
	void updateTextureFromFrame(AVFrame* frame);
	
	void resizeWindowFileLoaded(AVCodecContext* codecContext, const WindowSize* initialParams);
	void resetRescaler(AVCodecContext* codecContext, const WindowSize* initialParams);
	void clampLuminance(AVFrame* frame, uint8_t minLuminance);

    // Reset method: destroys SDL resources
    void reset();
    void cleanup();

    bool isReady();
    
    SDL_Window* window = nullptr;
    HWND hwnd = nullptr;
    AudioDevice* audioDevice = nullptr;

private:
    Uint32 window_id;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* videoTexture = nullptr;
    SDL_Texture* titleBarTexture = nullptr;
    
    bool initialized;
    bool started;
    
    int windowWidth = 0;
    int titleBarHeight = 31;
};


