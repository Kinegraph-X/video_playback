#pragma once

#include "player_headers.h"

#include <queue>
#include <mutex>
#include <condition_variable>
#include <libavcodec/avcodec.h>

class PacketQueue {
public:
    PacketQueue();
    ~PacketQueue();

    // Add a packet to the queue
    bool put(AVPacket* pkt);

    // Retrieve a packet from the queue
    bool get(AVPacket* pkt);
    
    size_t getSize();

    // Clear the queue
    void flush();

    // Signal all waiting threads to abort
    void setAbort(bool value);

    // Check if the queue is empty
    bool isEmpty() const ;

private:
    std::queue<AVPacket*> queue;              // Queue of AVPacket pointers
    mutable std::mutex mutex;                // Mutex for thread-safe access
    std::condition_variable condition;       // Condition variable for thread synchronization
    bool abort;                              // Abort flag to stop operations
};

