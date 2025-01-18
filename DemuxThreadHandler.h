#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>
#include <atomic>
#include "PacketQueue.h"
#include "AVFormatHandler.h"

class DemuxThreadHandler {
private:
    AVFormatHandler& formatHandler;
    PacketQueue& videoQueue;
    PacketQueue& audioQueue;
    std::atomic<bool> shouldRun{false};
    AVPacket* packet;
    
    mutable std::mutex threadMutex;
    std::condition_variable threadCondition;
    std::atomic<bool> abort = false;

public:
    DemuxThreadHandler(
		PacketQueue& videoQueue,
		PacketQueue& audioQueue,
		AVFormatHandler& formatHandler
	);
    ~DemuxThreadHandler();

    void demuxPackets(size_t maxVideoQueueSize, size_t maxAudioQueueSize);

    void stopThread();

    void wakeUpThread();

    PacketQueue& getVideoPacketQueue() { return videoQueue; }
    PacketQueue& getAudioPacketQueue() { return audioQueue; }
    
    void setAbort(bool value);
    bool isAborted();
    
    bool exhausted = false;
    void reset();

private:
    void triagePacket(AVPacket* packet);
    void cleanup();
};
