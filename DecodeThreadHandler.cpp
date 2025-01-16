#include <DecodeThreadHandler.h>


DecodeThreadHandler::DecodeThreadHandler(
	PacketQueue& videoPacketQueue,
	PacketQueue& audioPacketQueue,
	FrameQueue& videoQueue,
	FrameQueue& audioQueue,
	AVFormatHandler& formatHandler
) : videoPacketQueue(videoPacketQueue),
	audioPacketQueue(audioPacketQueue),
	videoQueue(videoQueue),
	audioQueue(audioQueue),
	formatHandler(formatHandler) {};
DecodeThreadHandler::~DecodeThreadHandler() {
	stopThread();
	av_packet_free(&packet);
	av_frame_free(&frame);
};

void DecodeThreadHandler::decodePackets(size_t maxVideoQueueSize, size_t maxAudioQueueSize) {
	packet = av_packet_alloc();
	if (!packet) {
        logger(LogLevel::ERR, "Failed to allocate AVPacket for decoding");
        return;
    }
	frame = av_frame_alloc();
	if (!frame) {
        logger(LogLevel::ERR, "Failed to allocate AVFrame for decoding");
        return;
    }
    
 	while (!isAborted()) {
		decodeVideoPackets(maxVideoQueueSize);
		decodeAudioPackets(maxAudioQueueSize);
		
		std::this_thread::sleep_for(std::chrono::microseconds(2));
	}
	logger(LogLevel::DEBUG, "Decode thread exited its infinite loop");
};

void DecodeThreadHandler::decodeVideoPackets(size_t maxQueueSize) {
	int ret;
	
	if (shouldRun.load(std::memory_order_acquire)
			&& videoPacketQueue.getSize() > 0 
			&& videoQueue.getSize() < maxQueueSize) {
				
		videoPacketQueue.get(packet);
		char errBuf[AV_ERROR_MAX_STRING_SIZE];
		
	    ret = avcodec_send_packet(formatHandler.getVideoCodecContext(), packet);
	    if (ret < 0) {
			const char* errStr = av_make_error_string(errBuf, AV_ERROR_MAX_STRING_SIZE, ret);
			logger(LogLevel::ERR, std::string("Error submitting a video packet for decoding : ") + std::string(errStr));
			av_packet_unref(packet);
			return;
	    }
	 
	    while (ret >= 0) {
	        ret = avcodec_receive_frame(formatHandler.getVideoCodecContext(), frame);
	        if (ret == AVERROR(EAGAIN)) {
				// We might have not enough video data in packet (this happens after each succesfully decoded frame)
	            continue;
	        }
	        else if (ret == AVERROR_EOF) {
				const char* errStr = av_make_error_string(errBuf, AV_ERROR_MAX_STRING_SIZE, ret);
	            logger(LogLevel::ERR, std::string("We have reached the end of the file (video decode part) : ") + std::string(errStr));
	            continue;
			}
	        else if (ret < 0) {
				const char* errStr = av_make_error_string(errBuf, AV_ERROR_MAX_STRING_SIZE, ret);
				logger(LogLevel::ERR, std::string("Unknown Error while decoding a video packet : ") + std::string(errStr));
				continue;
			}
	 		
//	 		logger(LogLevel::INFO, std::string("video frame pts : ") + LogUtils::toString(frame->best_effort_timestamp));
	 		videoQueue.put(frame);
	 	}
	 	
	 	av_packet_unref(packet);
		av_frame_unref(frame);
    }
    else if (shouldRun.load(std::memory_order_acquire) && videoPacketQueue.isEmpty()) {
		logger(LogLevel::INFO, std::string("decode Thread Self-Stopped (video packetQueue is empty)"));
		logger(LogLevel::INFO, std::string("audioFrameQueue size : ") + LogUtils::toString(audioQueue.getSize()));
		logger(LogLevel::INFO, std::string("videoFrameQueue size : ") + LogUtils::toString(videoQueue.getSize()));
		shouldRun = false;
			
		av_packet_unref(packet);
		av_frame_unref(frame);
	}
    else if (shouldRun.load(std::memory_order_acquire)) {
//		logger(LogLevel::INFO, std::string("Video packet queue size : ") + LogUtils::toString(videoPacketQueue.getSize()));
//		logger(LogLevel::INFO, std::string("Video frame queue size : ") + LogUtils::toString(videoQueue.getSize()));
//		logger(LogLevel::INFO, std::string("max queue size : ") + LogUtils::toString(maxQueueSize));
//		logger(LogLevel::INFO, std::string("condition is : ") + LogUtils::toString(shouldRun.load(std::memory_order_acquire) && videoPacketQueue.getSize() > 0 && videoQueue.getSize() < maxQueueSize));
	
//		logger(LogLevel::INFO, std::string("audioFrameQueue size : ") + LogUtils::toString(audioQueue.getSize()));
//		logger(LogLevel::INFO, std::string("videoFrameQueue size : ") + LogUtils::toString(videoQueue.getSize()));
//		shouldRun = false;
//		logger(LogLevel::INFO, std::string("decode Thread Self-Stopped"));
			
		av_packet_unref(packet);
		av_frame_unref(frame);
	}
}

void DecodeThreadHandler::decodeAudioPackets(size_t maxQueueSize) {
	int ret;
	
	if (shouldRun.load(std::memory_order_acquire)
			&& audioPacketQueue.getSize() > 0 
			&& audioQueue.getSize() < maxQueueSize) {
		
		audioPacketQueue.get(packet);
		char errBuf[AV_ERROR_MAX_STRING_SIZE];
		
//		logger(LogLevel::INFO, std::string("audio packet from queue : size : ") + LogUtils::toString(packet->size));
//		logger(LogLevel::INFO, std::string("audio packet from queue : pts : ") + LogUtils::toString(packet->pts));
//		logger(LogLevel::INFO, std::string("audio packet from queue : dts : ") + LogUtils::toString(packet->dts));
//		logger(LogLevel::INFO, std::string("audio packet from queue : stream_index : ") + LogUtils::toString(packet->stream_index));
	 
	    ret = avcodec_send_packet(formatHandler.getAudioCodecContext(), packet);
	    if (ret < 0) {
			const char* errStr = av_make_error_string(errBuf, AV_ERROR_MAX_STRING_SIZE, ret);
			logger(LogLevel::ERR, std::string("Error submitting an audio packet for decoding : ") + std::string(errStr));
			av_packet_unref(packet);
			return;
	    }
	 
	    while (ret >= 0) {
	        ret = avcodec_receive_frame(formatHandler.getAudioCodecContext(), frame);
	        if (ret == AVERROR(EAGAIN)) {
				// We might have not enough audio data in packet (this happens after each succesfully decoded frame)
	            continue;
	        }
	        else if (ret == AVERROR_EOF) {
				const char* errStr = av_make_error_string(errBuf, AV_ERROR_MAX_STRING_SIZE, ret);
	            logger(LogLevel::ERR, std::string("We have reached the end of the file (audio decode part) : ") + std::string(errStr));
	            continue;
			}
	        else if (ret < 0) {
				const char* errStr = av_make_error_string(errBuf, AV_ERROR_MAX_STRING_SIZE, ret);
				logger(LogLevel::ERR, std::string("Unknown Error while decoding an audio packet : ") + std::string(errStr));
				continue;
			}
			
//			logger(LogLevel::INFO, std::string("audio frame pts : ") + LogUtils::toString(frame->best_effort_timestamp));
	 		
	 		audioQueue.put(frame);
	 	}
	 	av_packet_unref(packet);
 		av_frame_unref(frame);
	}
	else if (shouldRun.load(std::memory_order_acquire) && audioPacketQueue.isEmpty()) {
		logger(LogLevel::INFO, std::string("decode Thread Self-Stopped (audio packetQueue is empty)"));
		logger(LogLevel::INFO, std::string("audioFrameQueue size : ") + LogUtils::toString(audioQueue.getSize()));
		logger(LogLevel::INFO, std::string("videoFrameQueue size : ") + LogUtils::toString(videoQueue.getSize()));
		shouldRun = false;
			
		av_packet_unref(packet);
		av_frame_unref(frame);
	}
	else if (shouldRun.load(std::memory_order_acquire)) {
//		logger(LogLevel::INFO, std::string("Video packet queue size : ") + LogUtils::toString(videoPacketQueue.getSize()));
//		logger(LogLevel::INFO, std::string("Video frame queue size : ") + LogUtils::toString(videoQueue.getSize()));
//		logger(LogLevel::INFO, std::string("max queue size : ") + LogUtils::toString(maxQueueSize));
//		logger(LogLevel::INFO, std::string("condition is : ") + LogUtils::toString(shouldRun.load(std::memory_order_acquire) && videoPacketQueue.getSize() > 0 && videoQueue.getSize() < maxQueueSize));
		logger(LogLevel::INFO, std::string("decode Thread Self-Stopped"));
		logger(LogLevel::INFO, std::string("audioFrameQueue size : ") + LogUtils::toString(audioQueue.getSize()));
		logger(LogLevel::INFO, std::string("videoFrameQueue size : ") + LogUtils::toString(videoQueue.getSize()));
		shouldRun = false;
			
		av_packet_unref(packet);
		av_frame_unref(frame);
	}
};

void DecodeThreadHandler::stopThread() {
	shouldRun.store(false, std::memory_order_release);
	logger(LogLevel::INFO, std::string("Decode Thread Stop called"));
};

void DecodeThreadHandler::wakeUpThread() {
	shouldRun.store(true, std::memory_order_release);
//	logger(LogLevel::INFO, std::string("Decode Thread Start called"));
};

void DecodeThreadHandler::setAbort(bool value) {
	logger(LogLevel::DEBUG, "DecodeThreadHandler::setAbort CALLED");
	std::lock_guard<std::mutex> lock(mutex);
	abort = value;
}

bool DecodeThreadHandler::isAborted() {
	std::lock_guard<std::mutex> lock(mutex);
	return abort;
}

