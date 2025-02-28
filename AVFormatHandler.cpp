#include "AVFormatHandler.h"

// Constructor and Destructor
AVFormatHandler::AVFormatHandler() {}
AVFormatHandler::~AVFormatHandler() {
	cleanup();
}

// Open a video file and initialize contexts
bool AVFormatHandler::openFile(const char *filePath) {
//	this->formatContext =  avformat_alloc_context();
	std::string basePath = GetExecutablePath();
	std::string fullPath = basePath + filePath;
	
	if (avformat_open_input(&this->formatContext, filePath, NULL, NULL) != 0) {
		logger(LogLevel::ERR, std::string("FFMPEG failed to open the file ") + std::string(filePath));
		return false;
	}
	else {
		logger(LogLevel::INFO, std::string("FFMPEG successfully opened the file ") + std::string(filePath));
	}
	if (avformat_find_stream_info(this->formatContext, NULL) >= 0) {
		logger(LogLevel::INFO, std::string("FFMPEG Found stream info"));
	}
	if (this->openStreams() == true) {
		logger(LogLevel::INFO, std::string("codec params extradata ") + LogUtils::toString(*this->videoCodecPar->extradata));
		logger(LogLevel::INFO, std::string("video stream id ") + LogUtils::toString(this->videoStream->id));
		logger(LogLevel::INFO, std::string("audio stream id ") + LogUtils::toString(this->audioStream->id));
		return true;
	}
	return false;
}

bool AVFormatHandler::openStreams() {
	if (this->setVideoStream() == true
			&& this->setAudioStream() == true) {
		this->setAudioSampleRate();
		this->setAudioChannelLayout();
		this->setAudioSampleFormat();
		this->setFrameRate();
		this->setFrameDuration();
		this->setDuration();
		
		logger(LogLevel::INFO, std::string("audio samplerate is ") + LogUtils::toString(this->audioSampleRate));
		logger(LogLevel::INFO, std::string("video frame duration is ") + LogUtils::toString(this->videoFrameDuration));
		return true;
	}
	return false;
}

bool AVFormatHandler::setVideoStream() {
	AVCodecContext* codecContext = nullptr;
	if (this->initializeCodecContext(&this->videoStreamIndex, this->formatContext, &codecContext, AVMEDIA_TYPE_VIDEO) >= 0){
        this->videoStream = this->formatContext->streams[this->videoStreamIndex];
		this->videoCodecPar = this->videoStream->codecpar ;
        this->videoCodecContext = codecContext ;
        logger(LogLevel::INFO, std::string("video stream index is ") + LogUtils::toString(this->videoStreamIndex));
        
        return true;
    }
    return false;
}
bool AVFormatHandler::setAudioStream() {
	AVCodecContext* codecContext = nullptr;
	if (this->initializeCodecContext(&this->audioStreamIndex, this->formatContext, &codecContext, AVMEDIA_TYPE_AUDIO) >= 0){
        this->audioStream = this->formatContext->streams[this->audioStreamIndex];
        this->audioCodecPar = this->audioStream->codecpar ;
        this->audioCodecContext = codecContext ;
        logger(LogLevel::INFO, std::string("audio stream index is ") + LogUtils::toString(this->audioStreamIndex));
        
        return true;
    }
    return false;
}

int AVFormatHandler::initializeCodecContext(int *stream_idx, AVFormatContext *formatContext, AVCodecContext **codecContext, enum AVMediaType type) {
	int ret, stream_index;
	AVStream *stream;
	const AVCodec *decoder = nullptr;
	
	stream_index  = av_find_best_stream(formatContext, type, -1, -1, NULL, 0);
	if (stream_index  < 0) {
        fprintf(stderr, "Could not find %s stream in input file\n", av_get_media_type_string(type));
        return stream_index;
    }
    else {
		stream = formatContext->streams[stream_index];
		decoder = avcodec_find_decoder(stream->codecpar->codec_id);
		if (decoder->capabilities & AV_CODEC_CAP_DELAY) {
		    logger(LogLevel::DEBUG, streamTypeToString(type) + " : The codec supports delayed frames");
		}
		else {
			logger(LogLevel::DEBUG, streamTypeToString(type) + " : The codec DOES NOT supports delayed frames");
		}
		*codecContext = avcodec_alloc_context3(decoder);
		if (!decoder) {
			logger(LogLevel::ERR, std::string("Failed find a decoder"));
		}
		if (ret = avcodec_parameters_to_context(*codecContext, stream->codecpar) < 0) {
		    logger(LogLevel::ERR, std::string("Failed to copy codec parameters to context: "));
		}
		if ((ret = avcodec_open2(*codecContext, decoder, NULL)) < 0) {
            logger(LogLevel::ERR, std::string("Failed to open codec") + av_get_media_type_string(type));
            return ret;
        }
		
		// LOG what we've found
		char codecInfo[2048];
        avcodec_string(codecInfo, sizeof(codecInfo), *codecContext, 1);
		logger(LogLevel::INFO, av_get_media_type_string(type) + std::string(" codec context is ") + std::string(codecInfo));

        *stream_idx = stream_index;
	}
	return ret;
}

void AVFormatHandler::setAudioSampleRate() {
	this->audioSampleRate = this->audioCodecContext->sample_rate;
}

void AVFormatHandler::setFrameRate() {
	this->videoFrameRate = (double)this->videoCodecPar->framerate.num / this->videoCodecPar->framerate.den;
	logger(LogLevel::DEBUG, "AVFormatContext : video framerate : " + LogUtils::toString(this->videoFrameRate));
}

void AVFormatHandler::setFrameDuration() {
	this->videoFrameDuration = 1. / this->videoFrameRate;
	logger(LogLevel::DEBUG, "AVFormatContext : video frame duration : " + LogUtils::toString(this->videoFrameDuration));
}

void AVFormatHandler::setAudioChannelLayout() {
	this->audioChannelLayout = &this->audioCodecContext->ch_layout;
}

void AVFormatHandler::setAudioSampleFormat() {
	this->sampleFormat = this->audioCodecContext->sample_fmt;
}

void AVFormatHandler::setDuration() {
	this->duration = (double)this->videoStream->duration * this->videoStream->time_base.num / this->videoStream->time_base.den;
}

AVFormatContext* AVFormatHandler::getFormatContext(){return formatContext;}
AVCodecContext* AVFormatHandler::getVideoCodecContext() {return videoCodecContext;}
AVCodecContext* AVFormatHandler::getAudioCodecContext() {return audioCodecContext;}

int AVFormatHandler::getVideoStreamIndex() {return videoStreamIndex;};
int AVFormatHandler::getAudioStreamIndex() {return audioStreamIndex;};

// Retrieve stream time bases for synchronization
//AVRational AVFormatHandler::getVideoTimeBase() {};
//AVRational AVFormatHandler::getAudioTimeBase() {};

std::string AVFormatHandler::streamTypeToString(AVMediaType type) {
	switch(type) {
		case AVMEDIA_TYPE_VIDEO : 
			return "Video";
		case AVMEDIA_TYPE_AUDIO : 
			return "Audio";
		default:
			return "Unknown";
	}
}


// Reset and release all resources
void AVFormatHandler::reset() {
	cleanup();
}

// Helper methods
void AVFormatHandler::cleanup() {
	logger(LogLevel::DEBUG, "AVFormatHandler::cleanup called");
	if (videoCodecContext) {
		avcodec_free_context(&videoCodecContext);
		// videoCodecContext is now nullptr
	}
	logger(LogLevel::DEBUG, "AVFormatHandler::cleanup videoCodecContext freed");
	
	if (audioCodecContext) {
		avcodec_free_context(&audioCodecContext);
		// audioCodecContext is now nullptr
	}
	logger(LogLevel::DEBUG, "AVFormatHandler::cleanup audioCodecContext freed");
	
	if (formatContext) {
		avformat_close_input(&formatContext);
		// formatContext is now nullptr
	}
	logger(LogLevel::DEBUG, "AVFormatHandler::cleanup formatContext closed");
	
	// Reset other pointers
	videoStream = nullptr;
	audioStream = nullptr;
	videoCodecPar = nullptr;  // These are owned by formatContext
	audioCodecPar = nullptr;  // These are owned by formatContext
	audioChannelLayout = nullptr;  // If this is owned elsewhere, adjust accordingly
	
	// Reset primitive types
	videoStreamIndex = -1;
	audioStreamIndex = -1;
	audioSampleRate = 0;
	videoFrameRate = 0.;
	videoFrameDuration = 0.;
	sampleFormat = AV_SAMPLE_FMT_NONE;
	duration = 0;
	initialized = false;
	
	logger(LogLevel::DEBUG, "AVFormatHandler::cleanup sequence ended");
}


//