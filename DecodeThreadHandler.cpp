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
	cleanup();
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
		std::unique_lock<std::mutex> lock(threadMutex);
		threadCondition.wait(lock, [this]() { return shouldRun.load(std::memory_order_acquire) || isAborted();});
		
		if (isAborted()) {
	        break;
	    }
	    
		decodeVideoPackets(maxVideoQueueSize, lock);
		decodeAudioPackets(maxAudioQueueSize, lock);
		
		if (isAborted()) {
	        break;
	    }
		
		lock.unlock();
		std::this_thread::sleep_for(std::chrono::microseconds(2));
	}
	logger(LogLevel::DEBUG, "Decode thread exited its infinite loop");
};

void DecodeThreadHandler::decodeVideoPackets(size_t maxQueueSize, std::unique_lock<std::mutex>& lock) {
	int ret;
	
	if (videoPacketQueue.getSize() > 0 
		&& videoQueue.getSize() < maxQueueSize) {

		lock.unlock();
//		logger(LogLevel::DEBUG, "DECODE THREAD accessing the queue");
		if (!videoPacketQueue.get(packet)) {
			av_packet_unref(packet);
			return;
		}
		if (packet->size == 0) {
			packet->data = NULL;
			logger(LogLevel::DEBUG, "Found empty video packet, looks like end of file");
		}
//		logger(LogLevel::DEBUG, "DECODE THREAD accessed the queue" + LogUtils::toString(packet->pts));
		char errBuf[AV_ERROR_MAX_STRING_SIZE];
		
	    ret = avcodec_send_packet(formatHandler.getVideoCodecContext(), packet);
	    if (ret < 0) {
			const char* errStr = av_make_error_string(errBuf, AV_ERROR_MAX_STRING_SIZE, ret);
			logger(LogLevel::ERR, std::string("Error submitting a video packet for decoding : ") + std::string(errStr));
			debugPacket(packet);
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
			av_packet_unref(packet);
//			lock.lock();
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
	            setAbort(true);
	            exhausted = true;
	            continue;
			}
	        else if (ret < 0) {
				const char* errStr = av_make_error_string(errBuf, AV_ERROR_MAX_STRING_SIZE, ret);
				logger(LogLevel::ERR, std::string("Unknown Error while decoding a video packet : ") + std::string(errStr));
				continue;
			}
	 		
//	 		logger(LogLevel::INFO, std::string("video frame pts : ") + LogUtils::toString(frame->best_effort_timestamp));
	 		videoQueue.put(frame);
	 		av_frame_unref(frame);
	 	}
//	 	logger(LogLevel::DEBUG, "DECODE THREAD returned a frame" + LogUtils::toString(frame->pts));
	 	
	 	av_packet_unref(packet);
		av_frame_unref(frame);
		
		lock.lock();
    }
    else if (shouldRun.load(std::memory_order_acquire) && videoPacketQueue.isEmpty() && audioPacketQueue.isEmpty()) {
		logger(LogLevel::DEBUG, std::string("decode Thread Self-Stopped (both packetQueues are empty)"));
		logger(LogLevel::INFO, std::string("audioFrameQueue size : ") + LogUtils::toString(audioQueue.getSize()));
		logger(LogLevel::INFO, std::string("videoFrameQueue size : ") + LogUtils::toString(videoQueue.getSize()));
		shouldRun = false;
			
		av_packet_unref(packet);
		av_frame_unref(frame);
		threadCondition.notify_all();
	}
    else if (shouldRun.load(std::memory_order_acquire)) {
//		logger(LogLevel::INFO, std::string("decode Thread Self-Stopped"));
//		logger(LogLevel::INFO, std::string("audioFrameQueue size : ") + LogUtils::toString(audioQueue.getSize()));
//		logger(LogLevel::INFO, std::string("videoFrameQueue size : ") + LogUtils::toString(videoQueue.getSize()));
//		shouldRun = false;
			
		av_packet_unref(packet);
		av_frame_unref(frame);
		threadCondition.notify_all();
	}
}

void DecodeThreadHandler::decodeAudioPackets(size_t maxQueueSize, std::unique_lock<std::mutex>& lock) {
	int ret;
	
	if (audioPacketQueue.getSize() > 0 
		&& audioQueue.getSize() < maxQueueSize) {
				
		lock.unlock();
		
		if (!audioPacketQueue.get(packet)) {
			av_packet_unref(packet);
			return;
		}
		if (packet->size == 0) {
			packet->data = NULL;
			logger(LogLevel::DEBUG, "Found empty audio packet, looks like end of file");
		}
		char errBuf[AV_ERROR_MAX_STRING_SIZE];
		
	    ret = avcodec_send_packet(formatHandler.getAudioCodecContext(), packet);
	    if (ret < 0) {
			const char* errStr = av_make_error_string(errBuf, AV_ERROR_MAX_STRING_SIZE, ret);
			logger(LogLevel::ERR, std::string("Error submitting an audio packet for decoding : ") + std::string(errStr));
			debugPacket(packet);
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
			av_packet_unref(packet);
//			lock.lock();
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
	            setAbort(true);
	            exhausted = true;
	            continue;
			}
	        else if (ret < 0) {
				const char* errStr = av_make_error_string(errBuf, AV_ERROR_MAX_STRING_SIZE, ret);
				logger(LogLevel::ERR, std::string("Unknown Error while decoding an audio packet : ") + std::string(errStr));
				continue;
			}
			
//			logger(LogLevel::INFO, std::string("audio frame pts : ") + LogUtils::toString(frame->best_effort_timestamp));
	 		
	 		audioQueue.put(frame);
	 		av_frame_unref(frame);
	 	}
	 	av_packet_unref(packet);
 		av_frame_unref(frame);
 		
 		lock.lock();
	}
	else if (shouldRun.load(std::memory_order_acquire) && audioPacketQueue.isEmpty() && videoPacketQueue.isEmpty()) {
		logger(LogLevel::DEBUG, std::string("decode Thread Self-Stopped (both packetQueue are empty)"));
		logger(LogLevel::INFO, std::string("audioFrameQueue size : ") + LogUtils::toString(audioQueue.getSize()));
		logger(LogLevel::INFO, std::string("videoFrameQueue size : ") + LogUtils::toString(videoQueue.getSize()));
		shouldRun = false;
			
		av_packet_unref(packet);
		av_frame_unref(frame);
		threadCondition.notify_all();
	}
	else if (shouldRun.load(std::memory_order_acquire)) {
		logger(LogLevel::INFO, std::string("decode Thread Self-Stopped"));
		logger(LogLevel::INFO, std::string("audioFrameQueue size : ") + LogUtils::toString(audioQueue.getSize()));
		logger(LogLevel::INFO, std::string("videoFrameQueue size : ") + LogUtils::toString(videoQueue.getSize()));
		shouldRun = false;
			
		av_packet_unref(packet);
		av_frame_unref(frame);
		threadCondition.notify_all();
	}
};

//void DecodeThreadHandler::stopThread() {
//	std::unique_lock<std::mutex> lock(threadMutex);
//	if (!isAborted()) {
//	    shouldRun.store(false, std::memory_order_release);
//	    threadCondition.notify_all();
//	    // Wait until the current iteration ends
//	    threadCondition.wait(lock, [this]() { return !shouldRun.load(std::memory_order_acquire); });
//    }
//	logger(LogLevel::DEBUG, std::string("Decode Thread Stop called"));
//};

void DecodeThreadHandler::stopThread() {
//    logger(LogLevel::DEBUG, "DecodeThread::stopThread - Starting");
    std::unique_lock<std::mutex> lock(threadMutex);
    if (!isAborted()) {
//        logger(LogLevel::DEBUG, "DecodeThread::stopThread - Setting shouldRun to false");
        shouldRun.store(false, std::memory_order_release);
//        logger(LogLevel::DEBUG, "DecodeThread::stopThread - Notifying thread to check shouldRun");
        threadCondition.notify_all();
//        logger(LogLevel::DEBUG, "DecodeThread::stopThread - Waiting for thread to acknowledge shouldRun=false");
        threadCondition.wait(lock, [this]() { return !shouldRun.load(std::memory_order_acquire); });
//        logger(LogLevel::DEBUG, "DecodeThread::stopThread - Thread acknowledged");
    } else {
//        logger(LogLevel::DEBUG, "DecodeThread::stopThread - Thread already aborted");
    }
    logger(LogLevel::DEBUG, "DecodeThread::stopThread - Completed");
}

void DecodeThreadHandler::wakeUpThread() {
	std::unique_lock<std::mutex> lock(threadMutex);
	shouldRun.store(true, std::memory_order_release);
	threadCondition.notify_all();
//	logger(LogLevel::INFO, std::string("Decode Thread Start called"));
};

void DecodeThreadHandler::setAbort(bool value) {
	std::lock_guard<std::mutex> lock(threadMutex);
	abort.store(value, std::memory_order_release);;
	threadCondition.notify_all();
	logger(LogLevel::DEBUG, "DecodeThreadHandler::setAbort called");
}

bool DecodeThreadHandler::isAborted() {
	return abort;
}

void DecodeThreadHandler::cleanup() {
	logger(LogLevel::DEBUG, "DecodeThreadHandler::cleanup started.");
    stopThread();
	av_packet_free(&packet);
	av_frame_free(&frame);
	logger(LogLevel::DEBUG, "DecodeThreadHandler::cleanup ended.");
}

void DecodeThreadHandler::reset() {
    cleanup();  // Call cleanup to reset resources
}

std::string packetFlagsToString(int flags) {
    std::string result;
    if (flags & AV_PKT_FLAG_KEY) result += "KEY ";
    if (flags & AV_PKT_FLAG_CORRUPT) result += "CORRUPT ";
    return result.empty() ? "NONE" : result;
}

void DecodeThreadHandler::debugPacket(const AVPacket *pkt) {

    logger(LogLevel::DEBUG, "Packet Debug Info:");

    // Packet data and size
    std::string hasData = pkt->data ? "Non-NULL" : "NULL";
    logger(LogLevel::DEBUG, "  pkt->data: " + hasData);
    logger(LogLevel::DEBUG, "  pkt->size: " + LogUtils::toString(pkt->size));

    // Stream index
    logger(LogLevel::DEBUG, "  pkt->stream_index: " + LogUtils::toString(pkt->stream_index));

    // Presentation timestamp (PTS)
    logger(LogLevel::DEBUG, "  pkt->pts: " + LogUtils::toString(pkt->pts));

    // Decompression timestamp (DTS)
    logger(LogLevel::DEBUG, "  pkt->dts: " + LogUtils::toString(pkt->dts));

    // Duration
    logger(LogLevel::DEBUG, "  pkt->duration: " + LogUtils::toString(pkt->duration));

    // Flags
    logger(LogLevel::DEBUG, "  pkt->flags: " + packetFlagsToString(pkt->flags));

    // Position in the stream (file position)
    logger(LogLevel::DEBUG, "  pkt->pos: " + LogUtils::toString(pkt->pos));

}


