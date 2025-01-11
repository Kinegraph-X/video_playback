#include "SDLAudioDevice.h"

AudioDevice::AudioDevice() {};
AudioDevice::~AudioDevice() {};

bool AudioDevice::startAudioDevice(int audio_sample_rate) {
	SDL_zero(this->desiredSpec);
	this->desiredSpec.freq = audio_sample_rate; //vs->audio_dec_ctx->sample_rate;
	this->desiredSpec.format = AUDIO_S16SYS;
	this->desiredSpec.channels = 2;
	this->desiredSpec.samples = 4096;
	this->deviceID = SDL_OpenAudioDevice(NULL, 0, &this->desiredSpec, &this->availableSpec, 0);
	SDL_PauseAudioDevice(deviceID, 0);
	return true;
};

		
