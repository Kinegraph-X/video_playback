#include "SDLAudioDevice.h"

AudioDevice::AudioDevice() {};
AudioDevice::~AudioDevice() {
    this->cleanup();
};

bool AudioDevice::startAudioDevice() {
	logger(LogLevel::DEBUG, "Number of audio devices available : " + std::to_string( SDL_GetNumAudioDevices(0)));
	
	SDL_zero(this->desiredSpec);
	this->desiredSpec.freq = SAMPLERATE;
	this->desiredSpec.format = AUDIO_S16SYS;
	this->desiredSpec.channels = CHANNEL_COUNT;
	this->desiredSpec.samples = BUFFER_SIZE;
	this->deviceID = SDL_OpenAudioDevice(NULL, 0, &this->desiredSpec, &this->availableSpec, 0);
	
	logger(LogLevel::DEBUG, "Audio buffer size obtained on the device : " + LogUtils::toString(this->availableSpec.samples));
	
	std::string lastError = std::string(SDL_GetError());
	if (lastError.empty()) {
		logger(LogLevel::INFO, "SDL_OpenAudioDevice succeeded");
	}
	else {
		logger(LogLevel::ERR, "SDL_OpenAudioDevice failed with error : " + lastError);
	}
	
	SDL_PauseAudioDevice(deviceID, 0);
	
	return true;
};

int AudioDevice::setSWRContext(AVFormatHandler* formatContext) {
	if (swrContext) {
		logger(LogLevel::DEBUG, "swrContext already exists");
		swr_free(&swrContext);
	}
	
	int ret;
	char errBuf[AV_ERROR_MAX_STRING_SIZE];
	av_channel_layout_default(&this->stereoChannelLayout, this->availableSpec.channels);

	swrContext = swr_alloc();
	if (!swrContext) {
		logger(LogLevel::ERR, "swr_alloc failed");
	}
	ret = swr_alloc_set_opts2(
		&swrContext,
		&this->stereoChannelLayout,
		AV_SAMPLE_FMT_S16,
		this->availableSpec.freq,
		formatContext->audioChannelLayout,
		formatContext->sampleFormat,
		formatContext->audioSampleRate,
		0,
		NULL
	);
	if (ret < 0) {
		const char* errStr = av_make_error_string(errBuf, AV_ERROR_MAX_STRING_SIZE, ret);
		logger(LogLevel::ERR, std::string("Failed to set options to the swr Context : ") + std::string(errStr));
		return ret;
	}
	ret = swr_init(swrContext);
    if (ret < 0) {
		const char* errStr = av_make_error_string(errBuf, AV_ERROR_MAX_STRING_SIZE, ret);
		logger(LogLevel::ERR, std::string("Failed to initialize the swr Context : ") + std::string(errStr));
		return ret;
	}
	return 0;
}

int AudioDevice::queueFrame(AVFrame* frame, AVFormatHandler* formatContext) {
    // Calculate the number of output samples based on input samples and conversion ratio
    // When the input and output sample rates don’t align perfectly (e.g., 44.1 kHz to 48 kHz),
    // the resampler processes a fraction of an input sample for each output sample.
    // It needs to "save" the remaining fractional data for future output samples,
    // rather than rounding or truncating it, which would reduce quality.
    // Resampling often involves digital filters, like low-pass filters,
    // to prevent aliasing or distortion. These filters need to see both "past" and "future" samples
    // to calculate the correct output for the current sample.
    
    // this returns an int64_t but other functions expect an int
    int outSampleCount = av_rescale_rnd(
        swr_get_delay(swrContext, this->availableSpec.freq) + frame->nb_samples,
        this->availableSpec.freq,
        formatContext->audioSampleRate,
        AV_ROUND_UP
    );

    // Allocate buffer for resampled audio
    uint8_t** resampledData = nullptr;
    int ret = av_samples_alloc_array_and_samples(
        &resampledData,
        nullptr,
        2, // Assuming stereo output (2 channels)
        outSampleCount,
        AV_SAMPLE_FMT_S16, // Target sample format
        0
    );
    if (ret < 0) {
        return ret; // Return error if allocation fails
    }

    // Perform resampling
    int samplesConverted = swr_convert(
        swrContext,
        resampledData,       // Output buffer
        outSampleCount,      // Max number of output samples
        (const uint8_t**)frame->data, // Input buffer
        frame->nb_samples    // Number of input samples
    );
    if (samplesConverted < 0) {
        av_freep(&resampledData[0]);
        av_freep(&resampledData);
        return samplesConverted;
    }

    // Calculate the size of the resampled data in bytes
    int dataSize = av_samples_get_buffer_size(
        nullptr,
        2,                     // Number of output channels
        samplesConverted,      // Number of converted samples
        AV_SAMPLE_FMT_S16,     // Target sample format
        1                      // Alignment (default: 1)
    );
    if (dataSize < 0) {
        av_freep(&resampledData[0]);
        av_freep(&resampledData);
        return dataSize;
    }

    SDL_QueueAudio(this->deviceID, resampledData[0], dataSize);

    av_freep(&resampledData[0]);
    av_freep(&resampledData);

    return dataSize; // Return the size of the queued audio data
}

int AudioDevice::getQueuedAudioSize() {
    return SDL_GetQueuedAudioSize(this->deviceID);
}

void AudioDevice::printStatus() {
    switch (SDL_GetAudioDeviceStatus(this->deviceID))
    {
        case SDL_AUDIO_STOPPED: 
        	logger(LogLevel::DEBUG, std::string("AudioDevice status : ") + std::string("stopped"));
        	break;
        case SDL_AUDIO_PLAYING: 
			logger(LogLevel::DEBUG, std::string("AudioDevice status : ") + std::string("playing"));
			break;
        case SDL_AUDIO_PAUSED: 
        	logger(LogLevel::DEBUG, std::string("AudioDevice status : ") + std::string("paused"));
        	break;
        default: 
        	std::string("???");
        	break;
    }
}

void AudioDevice::cleanup() {
	logger(LogLevel::DEBUG, "AudioDevice::cleanup started.");
    av_channel_layout_uninit(&stereoChannelLayout);
	swr_free(&swrContext);
	SDL_PauseAudioDevice(deviceID, 1);
	SDL_CloseAudioDevice(deviceID);
	logger(LogLevel::DEBUG, "AudioDevice::cleanup ended.");
}

void AudioDevice::reset() {
    cleanup();
}

