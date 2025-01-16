#pragma once

#include "player_headers.h"
#include "SDLAudioDevice.h"

class SDLManager {
public:
    SDLManager();
    ~SDLManager();

    bool initialize();

    // sets up SDL_Window, SDL_Renderer, and SDL_Texture
    bool start(int xpos, int ypos, int width, int height, char title[]);

    void render(const uint8_t* y_plane, int y_pitch,
                const uint8_t* u_plane, int u_pitch,
                const uint8_t* v_plane, int v_pitch,
                int video_width, int video_height);
	
	

    // Reset method: destroys SDL resources
    void reset();
    void cleanUp();

    bool isReady();
    
    AudioDevice* audioDevice = nullptr;

private:
    SDL_Window* window = nullptr;
    Uint32 window_id;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;     // SDL texture for video frames
    
    bool initialized;
    bool started;
};


