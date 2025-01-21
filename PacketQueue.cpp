#include "PacketQueue.h"


PacketQueue::PacketQueue() : abort(false) {}
PacketQueue::~PacketQueue() {
    flush(); // Ensure all packets are cleared on destruction
}

// Add a packet to the queue
bool PacketQueue::put(AVPacket* pkt) {
    std::lock_guard<std::mutex> lock(mutex);

	if (abort) {
        return false; 
    }
    
    // Create a new packet and copy data
    AVPacket* packet = av_packet_alloc();
    if (!packet || av_packet_ref(packet, pkt) < 0) {
		logger(LogLevel::ERR, "Packet allocation on queue failed : Packet size is : " + LogUtils::toString(pkt->size));
        return false; // Allocation or copying failed
    }

    queue.push(packet);
    condition.notify_all(); // Notify waiting threads
    return true;
}

// Retrieve a packet from the queue
bool PacketQueue::get(AVPacket* pkt) {
    std::unique_lock<std::mutex> lock(mutex);
	
	// condition.wait_for(lock, std::chrono::seconds(5), [this] { return !queue.empty() || abort; });
    condition.wait(lock, [this] { return !queue.empty(); });

	if (abort) {
        return false; // Exit if the operation was aborted
    }
    
    AVPacket* packet = queue.front();
    queue.pop();
    av_packet_move_ref(pkt, packet);
    av_packet_free(&packet);

    return true;
}

size_t PacketQueue::getSize() {
	std::lock_guard<std::mutex> lock(mutex);
	return queue.size();
}

// Clear the queue
void PacketQueue::flush() {
    std::unique_lock<std::mutex> lock(mutex);

    while (!queue.empty()) {
        AVPacket* packet = queue.front();
        queue.pop();
        av_packet_free(&packet);
    }

    condition.notify_all(); // Wake up any waiting threads
}

// Signal all waiting threads to abort
void PacketQueue::setAbort(bool value) {
    std::unique_lock<std::mutex> lock(mutex);
    abort = value;
    condition.notify_all(); // Wake up threads
}

// Check if the queue is empty
bool PacketQueue::isEmpty() const {
    std::unique_lock<std::mutex> lock(mutex);
    return queue.empty();
}
