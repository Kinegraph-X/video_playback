#pragma once

const int MAX_CONNECTIONS = 5;
const int SAMPLERATE = 48000;
const int CHANNEL_COUNT = 2;
const int BUFFER_SIZE = 4096;

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
