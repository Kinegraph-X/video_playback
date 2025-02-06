#include "DOM/EventBatch.h"

EventBatch::EventBatch() {}
EventBatch::~EventBatch() {
	cleanup();
}
	
void EventBatch::addEvent(const UIEvent& event) {
//	logger(LogLevel::DEBUG, "Adding event ID: " + std::to_string(event.eventID));
    if (event.type == EventType::MouseMove) {
        // Replace existing MouseMove event for this node
        events[event.targetNode][EventType::MouseMove] = event;
    } else {
        // For other event types, we might want to keep all events
        events[event.targetNode][event.type] = event;
    }
}

std::vector<UIEvent> EventBatch::getEvents() {
    std::vector<UIEvent> result;
    for (const auto& nodePair : events) {
        for (const auto& eventPair : nodePair.second) {
//			logger(LogLevel::DEBUG, "Retrieving event ID: " + std::to_string(eventPair.second.eventID));
            result.push_back(eventPair.second);
        }
    }
    return result;
}

void EventBatch::clear() {
	for (auto& nodePair : events) {
        nodePair.second.clear();  // Ensure inner maps are cleared
    }
	events.clear();	
}

void EventBatch::cleanup() {
    events.clear();
}
