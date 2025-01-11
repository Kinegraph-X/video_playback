#include "FrameQueue.h"


FrameQueue::FrameQueue() : abort(false) {}
FrameQueue::~FrameQueue() {
    flush(); // Ensure all packets are cleared on destruction
}

// Add a frame to the queue
bool FrameQueue::put(AVFrame* pkt) {
    std::lock_guard<std::mutex> lock(mutex);

    if (abort) {
        return false; // If abort is flagged, reject new packets
    }

    // Create a new frame and copy data
    AVFrame* frame = av_frame_alloc();
    if (!frame || av_frame_ref(frame, pkt) < 0) {
        return false; // Allocation or copying failed
    }

    queue.push(frame);
    condition.notify_one(); // Notify waiting threads
    return true;
}

// Retrieve a frame from the queue
bool FrameQueue::get(AVFrame* pkt) {
    std::unique_lock<std::mutex> lock(mutex);

    condition.wait(lock, [this] { return !queue.empty(); });

    AVFrame* frame = queue.front();
    queue.pop();
    av_frame_move_ref(pkt, frame);
    av_frame_free(&frame);

    return true;
}

size_t FrameQueue::getSize() {
	return queue.size();
}

// Clear the queue
void FrameQueue::flush() {
    std::unique_lock<std::mutex> lock(mutex);

    while (!queue.empty()) {
        AVFrame* frame = queue.front();
        queue.pop();
        av_frame_free(&frame);
    }

    condition.notify_all(); // Wake up any waiting threads
}

// Signal all waiting threads to abort
void FrameQueue::setAbort(bool value) {
    std::unique_lock<std::mutex> lock(mutex);
    abort = value;
    condition.notify_all(); // Wake up threads
}

// Check if the queue is empty
bool FrameQueue::isEmpty() const {
    std::unique_lock<std::mutex> lock(mutex);
    return queue.empty();
}
