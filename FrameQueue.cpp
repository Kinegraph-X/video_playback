#include "FrameQueue.h"


FrameQueue::FrameQueue() : abort(false) {}
FrameQueue::~FrameQueue() {
    flush(); // Ensure all packets are cleared on destruction
}

// Add a frame to the queue
bool FrameQueue::put(AVFrame* frame) {
    std::lock_guard<std::mutex> lock(mutex);

//    if (abort) {
//        return false; // If abort is flagged, reject new packets
//    }

    // Create a new frame and copy data
    AVFrame* queuedFrame = av_frame_alloc();
    if (!queuedFrame || av_frame_ref(queuedFrame, frame) < 0) {
        return false; // Allocation or copying failed
    }

    queue.push(queuedFrame);
    condition.notify_all(); // Notify waiting threads
    return true;
}

// Retrieve a frame from the queue
bool FrameQueue::get(AVFrame* frame) {
    std::unique_lock<std::mutex> lock(mutex);

    condition.wait(lock, [this] { return !queue.empty(); });

    AVFrame* queuedFrame = queue.front();
    queue.pop();
    av_frame_move_ref(frame, queuedFrame);
    av_frame_free(&queuedFrame);

    return true;
}

size_t FrameQueue::getSize() {
	std::lock_guard<std::mutex> lock(mutex);
	return queue.size();
}

// Clear the queue
void FrameQueue::flush() {
    std::unique_lock<std::mutex> lock(mutex);

    while (!queue.empty()) {
        AVFrame* queuedFrame = queue.front();
        queue.pop();
        av_frame_free(&queuedFrame);
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
