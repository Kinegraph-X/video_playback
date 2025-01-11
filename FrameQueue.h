#pragma once

#include "player_headers.h"

#include <queue>
#include <mutex>
#include <condition_variable>
#include <libavcodec/avcodec.h>

class FrameQueue {
public:
    FrameQueue();
    ~FrameQueue();

    // Add a packet to the queue
    bool put(AVFrame* pkt);

    // Retrieve a packet from the queue
    bool get(AVFrame* pkt);
    
    size_t getSize();

    // Clear the queue
    void flush();

    // Signal all waiting threads to abort
    void setAbort(bool value);

    // Check if the queue is empty
    bool isEmpty() const ;

private:
    std::queue<AVFrame*> queue;              // Queue of AVFrame pointers
    mutable std::mutex mutex;                // Mutex for thread-safe access
    std::condition_variable condition;       // Condition variable for thread synchronization
    bool abort;                              // Abort flag to stop operations
};

