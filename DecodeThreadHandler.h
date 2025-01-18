#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>
#include <atomic>
#include "PacketQueue.h"
#include "FrameQueue.h"
#include "AVFormatHandler.h"

class DecodeThreadHandler {
private:
    AVFormatHandler& formatHandler;
	PacketQueue& videoPacketQueue;
	PacketQueue& audioPacketQueue;
	FrameQueue& videoQueue;
	FrameQueue& audioQueue;
    std::atomic<bool> shouldRun{false};
    
    AVPacket* packet = nullptr;
    AVFrame* frame = nullptr;
    
    mutable std::mutex threadMutex;
    std::condition_variable threadCondition;
    std::atomic<bool> abort = false;

public:
    DecodeThreadHandler(
		PacketQueue& videoPacketQueue,
		PacketQueue& audioPacketQueue,
		FrameQueue& videoQueue,
		FrameQueue& audioQueue,
		AVFormatHandler& formatHandler
	);
    ~DecodeThreadHandler();
	
    void decodePackets(size_t maxVideoQueueSize, size_t maxAudioQueueSize);
    void decodeVideoPackets(size_t maxQueueSize, std::unique_lock<std::mutex>& lock);
    void decodeAudioPackets(size_t maxQueueSize, std::unique_lock<std::mutex>& lock);

    void stopThread();

    void wakeUpThread();

//    PacketQueue& getVideoPacketQueue() { return videoPacketQueue; }
//    PacketQueue& getAudioPacketQueue() { return audioPacketQueue; }

	void setAbort(bool value);
	bool isAborted();
    void reset();

private:
    void cleanup();
};
