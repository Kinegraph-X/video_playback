#pragma once
#include "player_headers.h"
#include "Node.h"

struct UIEvent {
	static int globalEventID;  // Static counter
    int eventID;
    Node* targetNode;
    EventType type;
    EventPayload payload;
    
    // Default ctor
    UIEvent() : targetNode(nullptr), type(EventType::None), payload(EventType::None) {
		eventID = globalEventID++;
	}

    UIEvent(EventType eventType, Node* node) : targetNode(node), type(eventType), payload(eventType) {
		eventID = globalEventID++;
	}
};
