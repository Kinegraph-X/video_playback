#include <DemuxThreadHandler.h>

DemuxThreadHandler::DemuxThreadHandler(
	PacketQueue& videoQueue,
	PacketQueue& audioQueue,
	AVFormatHandler& formatHandler
	) : videoQueue(videoQueue), audioQueue(audioQueue), formatHandler(formatHandler) {}
DemuxThreadHandler::~DemuxThreadHandler() {
	stopThread();
	av_packet_free(&packet);
}


void DemuxThreadHandler::demuxPackets(size_t maxVideoQueueSize, size_t maxAudioQueueSize) {
	packet = av_packet_alloc();
	if (!packet) {
        logger(LogLevel::ERR, "Failed to allocate AVPacket for demuxing");
        return;
    }
    
    while (!isAborted()) {
		if (shouldRun.load(std::memory_order_acquire)
			&& (videoQueue.getSize() < maxVideoQueueSize
				||  audioQueue.getSize() < maxAudioQueueSize)) {
			if (!formatHandler.getFormatContext()) {
				logger(LogLevel::ERR, std::string("formatContext empty"));
				break;
			}
//			logger(LogLevel::INFO, std::string("audioQueue size : ") + LogUtils::toString(audioQueue.getSize()));
//			logger(LogLevel::INFO, std::string("videoQueue size : ") + LogUtils::toString(videoQueue.getSize()));
			
			int ret;

	        ret = av_read_frame(formatHandler.getFormatContext(), packet);
	        if (ret < 0) {
		        if (ret == AVERROR_EOF) {
					exhausted = true;
					logger(LogLevel::INFO, std::string("End of file reached by demux thread"));
					break;
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
	        triagePacket(packet);
	    }
	    else if (shouldRun.load(std::memory_order_acquire)) {
			logger(LogLevel::INFO, std::string("demux Thread Self-Stopped"));
			logger(LogLevel::INFO, std::string("audioPacketQueue size : ") + LogUtils::toString(audioQueue.getSize()));
			logger(LogLevel::INFO, std::string("videoPacketQueue size : ") + LogUtils::toString(videoQueue.getSize()));
			shouldRun = false;
		}
		
		std::this_thread::sleep_for(std::chrono::microseconds(2));
    }
    logger(LogLevel::DEBUG, "Demux thread exited its infinite loop");
}

void DemuxThreadHandler::stopThread() {
	shouldRun.store(false, std::memory_order_release);
	logger(LogLevel::INFO, std::string("Demux Thread Stop called"));
};

void DemuxThreadHandler::wakeUpThread() {
	shouldRun.store(true, std::memory_order_release);
//	logger(LogLevel::INFO, std::string("Demux Thread Start called"));
};


// Method to triage packets into audio or video queues
void DemuxThreadHandler::triagePacket(AVPacket* packet) {
    if (packet->stream_index == formatHandler.getVideoStreamIndex()) {
        videoQueue.put(packet);
    } else if (packet->stream_index == formatHandler.getAudioStreamIndex()) {
        audioQueue.put(packet);
    }
//    else {
		//check if the packet belongs to a stream we are interested in, otherwise skip it
		av_packet_unref(packet);
//	}
}

void DemuxThreadHandler::setAbort(bool value) {
	std::lock_guard<std::mutex> lock(mutex);
	logger(LogLevel::DEBUG, "DemuxThreadHandler::setAbort CALLED");
	abort = value;
}

bool DemuxThreadHandler::isAborted() {
	std::lock_guard<std::mutex> lock(mutex);
	return abort;
}

