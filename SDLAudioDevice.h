#pragma once

#include "player_headers.h"

class AudioDevice {
	public:
		AudioDevice();
		~AudioDevice();
		
		bool startAudioDevice(int audio_sample_rate);
		
		SDL_AudioSpec desiredSpec, availableSpec;
		SDL_AudioDeviceID deviceID;

};
