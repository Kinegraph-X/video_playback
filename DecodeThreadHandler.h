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

public:
    DecodeThreadHandler(
		PacketQueue& videoPacketQueue,
		PacketQueue& audioPacketQueue,
		FrameQueue& videoQueue,
		FrameQueue& audioQueue,
		AVFormatHandler& formatHandler
	);
    ~DecodeThreadHandler();

    void decodeVideoPackets(size_t maxQueueSize);
    void decodeAudioPackets(size_t maxQueueSize);

    void stopThread();

    void wakeUpThread();

//    PacketQueue& getVideoPacketQueue() { return videoPacketQueue; }
//    PacketQueue& getAudioPacketQueue() { return audioPacketQueue; }

private:
//    void triagePacket(AVPacket& packet);
};
