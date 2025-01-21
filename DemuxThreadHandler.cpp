#include <DemuxThreadHandler.h>

DemuxThreadHandler::DemuxThreadHandler(
	PacketQueue& videoQueue,
	PacketQueue& audioQueue,
	AVFormatHandler& formatHandler
	) : videoQueue(videoQueue), audioQueue(audioQueue), formatHandler(formatHandler) {}
DemuxThreadHandler::~DemuxThreadHandler() {
	cleanup();
}


void DemuxThreadHandler::demuxPackets(size_t maxVideoQueueSize, size_t maxAudioQueueSize) {
	int lastStreamIndexSeen = 0;
	emptyPacket = av_packet_alloc();

	packet = av_packet_alloc();
	if (!packet) {
        logger(LogLevel::ERR, "Failed to allocate AVPacket for demuxing");
        return;
    }
    
    while (!isAborted()) {
		std::unique_lock<std::mutex> lock(threadMutex);
		threadCondition.wait(lock, [this]() { return shouldRun.load(std::memory_order_acquire) || isAborted();});	//  
		
		if (isAborted()) {
	        break;
	    }
		
		if (videoQueue.getSize() < maxVideoQueueSize
				||  audioQueue.getSize() < maxAudioQueueSize) {
					
			if (!formatHandler.getFormatContext()) {
				logger(LogLevel::ERR, std::string("formatContext empty"));
				abort = true;
				threadCondition.notify_all();
				setAbort(true);
			}
			lock.unlock();
//			logger(LogLevel::INFO, std::string("audioQueue size : ") + LogUtils::toString(audioQueue.getSize()));
//			logger(LogLevel::INFO, std::string("videoQueue size : ") + LogUtils::toString(videoQueue.getSize()));
			
			int ret;

	        ret = av_read_frame(formatHandler.getFormatContext(), packet);
	        if (ret < 0) {
		        if (ret == AVERROR_EOF) {
					if (!exhausted) {
						logger(LogLevel::INFO, std::string("End of file reached by demux thread"));
					}
					initEmptyPacket(emptyPacket, lastStreamIndexSeen);
					triagePacket(emptyPacket); // The first packet flushes the decoder, the second one is needed to trigger AVERROR_EOF
					shouldRun = false;
					exhausted = true;
					av_packet_unref(packet);
					av_packet_unref(emptyPacket);
					setAbort(true);
		        }
		        else if (formatHandler.getFormatContext()->pb->error != 0) {
		            logger(LogLevel::ERR, std::string("FormatContext error") + LogUtils::toString(formatHandler.getFormatContext()->pb->error));
		        }
		        else {
		            logger(LogLevel::ERR, std::string("Unknown error in demux thread"));
		        }
		    }
	        
//	        logger(LogLevel::INFO, std::string("packet size is ") + LogUtils::toString(packet->size));

	        // Triage the packet to the appropriate queue
	        lastStreamIndexSeen = packet->stream_index;
	        triagePacket(packet);
	        
	        lock.lock();
	        
	    }
	    else if (shouldRun.load(std::memory_order_acquire)) {
			logger(LogLevel::INFO, std::string("demux Thread Self-Stopped"));
			logger(LogLevel::INFO, std::string("audioPacketQueue size : ") + LogUtils::toString(audioQueue.getSize()));
			logger(LogLevel::INFO, std::string("videoPacketQueue size : ") + LogUtils::toString(videoQueue.getSize()));
			shouldRun = false;
			threadCondition.notify_all();
		}
		
		if (isAborted()) {
	        break;
	    }
		
		if (!shouldRun.load(std::memory_order_acquire)) {
            // Notify stopThread() that the iteration has ended
            threadCondition.notify_all();
        }
        
		std::this_thread::sleep_for(std::chrono::microseconds(2));
    }
    logger(LogLevel::DEBUG, "Demux thread exited its infinite loop");
}

//void DemuxThreadHandler::stopThread() {
//	std::unique_lock<std::mutex> lock(threadMutex);
//	if (!isAborted()) {
//		shouldRun.store(false, std::memory_order_release);
//		threadCondition.wait(lock, [this]() { return !shouldRun.load(std::memory_order_acquire); });
//		threadCondition.notify_all();
//	}
//	logger(LogLevel::DEBUG, std::string("Demux Thread Stop called"));
//};

void DemuxThreadHandler::stopThread() {
//    logger(LogLevel::DEBUG, "DemuxThread::stopThread - Starting");
    std::unique_lock<std::mutex> lock(threadMutex);
    if (!isAborted()) {
//        logger(LogLevel::DEBUG, "DemuxThread::stopThread - Setting shouldRun to false");
        shouldRun.store(false, std::memory_order_release);
//        logger(LogLevel::DEBUG, "DemuxThread::stopThread - Waiting for thread to acknowledge shouldRun=false");
        threadCondition.wait(lock, [this]() { return !shouldRun.load(std::memory_order_acquire); });
//        logger(LogLevel::DEBUG, "DemuxThread::stopThread - Thread acknowledged, notifying all");
        threadCondition.notify_all();
    } else {
//        logger(LogLevel::DEBUG, "DemuxThread::stopThread - Thread already aborted");
    }
    logger(LogLevel::DEBUG, "DemuxThread::stopThread - Completed");
}

void DemuxThreadHandler::wakeUpThread() {
	std::unique_lock<std::mutex> lock(threadMutex);
	shouldRun.store(true, std::memory_order_release);
	threadCondition.notify_all();
//	logger(LogLevel::INFO, std::string("Demux Thread Start called"));
};


// Method to triage packets into audio or video queues
void DemuxThreadHandler::triagePacket(AVPacket* packet) {
    if (packet->stream_index == formatHandler.getVideoStreamIndex()) {
		if (packet->size == 0) {
			logger(LogLevel::DEBUG, "Put flush packet onto video packet queue");
		}
        videoQueue.put(packet);
    } else if (packet->stream_index == formatHandler.getAudioStreamIndex()) {
		if (packet->size == 0) {
			logger(LogLevel::DEBUG, "Put flush packet onto audio packet queue");
		}
        audioQueue.put(packet);
    }
    else if (packet->size == 0) {
		// This is not met, seems packets have default stream index at 0
		logger(LogLevel::DEBUG, "seen flush packet in Demux Thread");
		videoQueue.put(packet);
	}
//    else {
		//check if the packet belongs to a stream we are interested in, otherwise skip it
		av_packet_unref(packet);
//	}
}

void DemuxThreadHandler::initEmptyPacket(AVPacket* pkt, int lastStreamIndexSeen) {
	av_packet_unref(pkt);
	packet->data = NULL;
	packet->size = 0;
	packet->stream_index = lastStreamIndexSeen;
}

void DemuxThreadHandler::setAbort(bool value) {
	std::lock_guard<std::mutex> lock(threadMutex);
	abort.store(value, std::memory_order_release);;
	threadCondition.notify_all();
	logger(LogLevel::DEBUG, "DemuxThreadHandler::setAbort called");
}

bool DemuxThreadHandler::isAborted() {
	return abort;
}

void DemuxThreadHandler::cleanup() {
	logger(LogLevel::DEBUG, "DemuxThreadHandler::cleanup started.");
    stopThread();
	av_packet_free(&packet);
	av_packet_free(&emptyPacket);
	logger(LogLevel::DEBUG, "DemuxThreadHandler::cleanup ended.");
}

void DemuxThreadHandler::reset() {
    cleanup();  // Call cleanup to reset resources
}