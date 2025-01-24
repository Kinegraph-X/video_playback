#pragma once

const int MAX_CONNECTIONS = 5;
const int SAMPLERATE = 48000;
const int CHANNEL_COUNT = 2;
const int BUFFER_SIZE = 512;

struct InitialParams {
	int xPos;
	int yPos;
	int width;
	int height;
	int bytes_per_sample;
};

struct WindowSize {
	int width = 0;
	int height = 0;
	int xPos = 0;
	int yPos = 0;
};

struct Socket_Params {
	unsigned short port;
};

struct PlayerEvent {
    enum Type { SHOULD_RENDER, SEEK_COMPLETE, PLAYBACK_FINISHED } type;
    double elapsedPlaybackTime;
};
