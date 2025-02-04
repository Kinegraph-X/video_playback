#include "DOM/EventQueue.h"

void EventQueue::pushEvent(const UIEvent& event) {
    std::lock_guard<std::mutex> lock(queueMutex);
    eventQueue.push(event);
}

EventBatch EventQueue::createBatch() {
    std::lock_guard<std::mutex> lock(queueMutex);
    EventBatch batch;
    while (!eventQueue.empty()) {
        batch.addEvent(eventQueue.front());
        eventQueue.pop();
    }
    return batch;
}

void EventQueue::populateBatch(EventBatch& batch) {
    std::lock_guard<std::mutex> lock(queueMutex);
    while (!eventQueue.empty()) {
        batch.addEvent(eventQueue.front());
        eventQueue.pop();
    }
}