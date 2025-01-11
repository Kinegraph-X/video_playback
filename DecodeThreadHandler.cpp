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
DecodeThreadHandler::~DecodeThreadHandler() {};

void DecodeThreadHandler::decodeVideoPackets(size_t maxQueueSize) {
	int ret;
	AVPacket* pkt = nullptr;
	AVFrame* frame = nullptr;
	videoPacketQueue.get(pkt);
	
 	while (true) {
		if (shouldRun.load(std::memory_order_relaxed)
				&& videoQueue.getSize() < maxQueueSize) {
		    ret = avcodec_send_packet(formatHandler.getVideoCodecContext(), pkt);
		//    if (ret < 0) {
		//        fprintf(stderr, "Error sending a packet for decoding\n");
		//        exit(1);
		//    }
		 
		    while (ret >= 0) {
		        ret = avcodec_receive_frame(formatHandler.getVideoCodecContext(), frame);
		        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
		            return;
		        else if (ret < 0) {
		            fprintf(stderr, "Error during decoding\n");
		            exit(1);
		        }
		 		
		 		videoQueue.put(frame);
		 	}
	    }
	}
};

void DecodeThreadHandler::decodeAudioPackets(size_t maxQueueSize) {
	int ret;
	AVPacket* pkt = nullptr;
	AVFrame* frame = nullptr;
	
	while (true) {
		if (shouldRun.load(std::memory_order_relaxed)
				&& audioQueue.getSize() < maxQueueSize) {
			audioPacketQueue.get(pkt);
		 
		    ret = avcodec_send_packet(formatHandler.getAudioCodecContext(), pkt);
		//    if (ret < 0) {
		//        fprintf(stderr, "Error sending a packet for decoding\n");
		//        exit(1);
		//    }
		 
		    while (ret >= 0) {
		        ret = avcodec_receive_frame(formatHandler.getAudioCodecContext(), frame);
		        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
		            return;
		        else if (ret < 0) {
		            fprintf(stderr, "Error during decoding\n");
		            exit(1);
		        }
		 		
		 		audioQueue.put(frame);
		 	}
		 }
 	}
};

void DecodeThreadHandler::stopThread() {
	shouldRun = false;
//    cv.notify_all();
};

void DecodeThreadHandler::wakeUpThread() {
	shouldRun = true;
//    cv.notify_all();
};

//    void triagePacket(AVPacket& packet);
