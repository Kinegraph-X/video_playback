#include <DemuxThreadHandler.h>

DemuxThreadHandler::DemuxThreadHandler(
	PacketQueue& videoQueue,
	PacketQueue& audioQueue,
	AVFormatHandler& formatHandler
	) : videoQueue(videoQueue), audioQueue(audioQueue), formatHandler(formatHandler) {}
DemuxThreadHandler::~DemuxThreadHandler() {
	stopThread();
	av_packet_unref(packet);
}


void DemuxThreadHandler::demuxPackets(size_t maxVideoQueueSize, size_t maxAudioQueueSize) {
	packet = av_packet_alloc();
	if (!packet) {
        logger(LogLevel::ERR, "Failed to allocate AVPacket");
        return;
    }
    
    while (true) {
		if (shouldRun.load(std::memory_order_relaxed)
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
					logger(LogLevel::ERR, std::string("End of file reached by demux thread"));
					break;
		        }
		        else if (formatHandler.getFormatContext()->pb->error != 0) {
		            logger(LogLevel::ERR, std::string("FormatContext error") + LogUtils::toString(formatHandler.getFormatContext()->pb->error));
		        }
		        else {
		            logger(LogLevel::ERR, std::string("Unknown error in demux thread"));
		            break;
		        }
		    }
	        
//	        logger(LogLevel::INFO, std::string("packet size is ") + LogUtils::toString(packet->size));

	        // Triage the packet to the appropriate queue
	        triagePacket(packet);
	    }
	    else if (shouldRun.load(std::memory_order_relaxed)) {
			logger(LogLevel::INFO, std::string("audioQueue size : ") + LogUtils::toString(audioQueue.getSize()));
			logger(LogLevel::INFO, std::string("videoQueue size : ") + LogUtils::toString(videoQueue.getSize()));
			shouldRun = false;
			logger(LogLevel::INFO, std::string("demuxThread Self-Stopped"));
		}
    }
}

// Stop the thread and notify waiting threads
void DemuxThreadHandler::stopThread() {
    shouldRun = false;
    logger(LogLevel::INFO, std::string("demuxThread Stopped"));
}

// Wake up the demux thread to start processing
void DemuxThreadHandler::wakeUpThread() {
    shouldRun = true;
}


// Method to triage packets into audio or video queues
void DemuxThreadHandler::triagePacket(AVPacket* packet) {
    if (packet->stream_index == formatHandler.getVideoStreamIndex()) {
        videoQueue.put(packet);
    } else if (packet->stream_index == formatHandler.getAudioStreamIndex()) {
        audioQueue.put(packet);
    } 
}

