#pragma once

#include "player_headers.h"
#include "AVFormatHandler.h"

class AudioDevice {
	public:
		AudioDevice();
		~AudioDevice();
		
		bool startAudioDevice();
		int queueFrame(AVFrame* frame, AVFormatHandler* formatContext);
		int setSWRContext(AVFormatHandler* formatContext);
		int getQueuedAudioSize();
		void printStatus();
		void reset();
		
		SDL_AudioSpec desiredSpec, availableSpec;
		SDL_AudioDeviceID deviceID;
		AVChannelLayout stereoChannelLayout;
		SwrContext* swrContext = nullptr;
	private :
		void cleanup();
};
