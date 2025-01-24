#include "EventListener.h"

void EventListener::initEventType(EventType type) {
    if (eventHandlers.find(type) == eventHandlers.end()) {
        eventHandlers[type] = std::vector<EventHandler>();
    }
}

void EventListener::addEventListener(EventType type, EventHandler handler) {
    initEventType(type);
    eventHandlers[type].push_back(handler);
}

void EventListener::removeEventListener(EventType type, EventHandler handler) {
    auto it = eventHandlers.find(type);
    if (it != eventHandlers.end()) {
        auto& handlers = it->second;
        handlers.erase(std::remove(handlers.begin(), handlers.end(), handler), handlers.end());
    }
}

void EventListener::dispatchEvent(const EventPayload& payload) {
    auto it = eventHandlers.find(payload.type);
    if (it != eventHandlers.end()) {
        for (const auto& handler : it->second) {
            handler(payload);
        }
    }
}

