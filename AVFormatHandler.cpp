#include "AVFormatHandler.h"

// Constructor and Destructor
AVFormatHandler::AVFormatHandler() {}
AVFormatHandler::~AVFormatHandler() {
	cleanup();
}

// Open a video file and initialize contexts
bool AVFormatHandler::openFile(const char *filePath) {
//	this->formatContext =  avformat_alloc_context();
	
	if (avformat_open_input(&this->formatContext, filePath, NULL, NULL) != 0) {
		logger(LogLevel::ERR, std::string("failed to open the file ") + std::string(filePath));
		return false;
	}
	else {
		logger(LogLevel::INFO, std::string("successfully opened the file ") + std::string(filePath));
	}
	if (avformat_find_stream_info(this->formatContext, NULL) >= 0) {
		logger(LogLevel::INFO, std::string("Found stream info"));
	}
	if (this->openStreams() == true) {
		logger(LogLevel::INFO, std::string("video stream id ") + LogUtils::toString(this->videoStream->id));
		logger(LogLevel::INFO, std::string("audio stream id ") + LogUtils::toString(this->audioStream->id));
		return true;
	}
	return false;
}

bool AVFormatHandler::openStreams() {
	if (this->getVideoStream() == true
			&& this->getAudioStream() == true) {
		this->getAudioSampleRate();
		this->getFrameDuration();
		
		logger(LogLevel::INFO, std::string("audio samplerate is ") + LogUtils::toString(this->audioSampleRate));
		logger(LogLevel::INFO, std::string("video frame duration is ") + LogUtils::toString(this->videoFrameDuration));
		return true;
	}
	return false;
}

bool AVFormatHandler::getVideoStream() {
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
bool AVFormatHandler::getAudioStream() {
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
		*codecContext = avcodec_alloc_context3(decoder);
		if (!decoder) {
			logger(LogLevel::ERR, std::string("Failed find a decoder"));
		}
		if ((ret = avcodec_open2(*codecContext, decoder, NULL)) < 0) {
            logger(LogLevel::ERR, std::string("Failed to open codec") + av_get_media_type_string(type));
            return ret;
        }
        if (ret = avcodec_parameters_to_context(*codecContext, stream->codecpar) < 0) {
		    logger(LogLevel::ERR, std::string("Failed to copy codec parameters to context: "));
		}
		
		// LOG what we've found
		char codecInfo[2048];
        avcodec_string(codecInfo, sizeof(codecInfo), *codecContext, 1);
		logger(LogLevel::INFO, av_get_media_type_string(type) + std::string(" codec context is ") + std::string(codecInfo));

        *stream_idx = stream_index;
	}
	return ret;
}

void AVFormatHandler::getAudioSampleRate() {
	this->audioSampleRate = this->audioCodecContext->sample_rate;
}

void AVFormatHandler::getFrameDuration() {
	this->videoFrameDuration = 1. / (double)this->videoCodecPar->framerate.num; 
}

AVFormatContext* AVFormatHandler::getFormatContext(){return formatContext;}
AVCodecContext* AVFormatHandler::getVideoCodecContext() {return videoCodecContext;}
AVCodecContext* AVFormatHandler::getAudioCodecContext() {return audioCodecContext;}

int AVFormatHandler::getVideoStreamIndex() {return videoStreamIndex;};
int AVFormatHandler::getAudioStreamIndex() {return audioStreamIndex;};

// Retrieve stream time bases for synchronization
//AVRational AVFormatHandler::getVideoTimeBase() {};
//AVRational AVFormatHandler::getAudioTimeBase() {};



// Check if the handler is ready
bool AVFormatHandler::isReady() {return true;}
// Reset and release all resources
void AVFormatHandler::reset() {}

// Helper methods
void AVFormatHandler::cleanup() {
	avformat_free_context(formatContext);
	avcodec_close(this->videoCodecContext);
	avcodec_close(this->audioCodecContext);
	avformat_close_input(&formatContext);
}


//