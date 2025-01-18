#pragma once

struct InitialParams {
	int xpos;
	int ypos;
	int width;
	int height;
	int bytes_per_sample;
};

struct WindowSize {
	int width = 0;
	int height = 0;
};

struct Socket_Params {
	int port;
};

struct PlayerEvent {
    enum Type { SHOULD_RENDER, SEEK_COMPLETE, PLAYBACK_FINISHED } type;
    double elapsedPlaybackTime;
};
