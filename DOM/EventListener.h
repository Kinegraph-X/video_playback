#pragma once

#include "../player_headers.h"
#include "EventType.h"
#include "EventPayload.h"

using EventHandler = std::function<void(const EventPayload&)>;

class EventListener {
private:
    std::map<EventType, std::vector<EventHandler>> eventHandlers;

public:
    void initEventType(EventType type);

    void addEventListener(EventType type, EventHandler handler);

    void removeEventListener(EventType type, EventHandler handler);
    
    void dispatchEvent(const EventPayload& payload);
};

