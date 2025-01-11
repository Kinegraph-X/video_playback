#pragma once

#include "player_headers.h"

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

    // Audio queue method: feeds audio data to the SDL audio queue
    void queueAudio(const uint8_t* audio_data, uint32_t length);

    // Reset method: destroys SDL resources
    void reset();

    bool isReady();

private:
    SDL_Window* window = nullptr;
    Uint32 window_id;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;     // SDL texture for video frames
    SDL_AudioDeviceID audioDevice;

    bool initialized;
    bool started;

    void cleanup();
};


