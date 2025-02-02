#pragma once
#include "player_headers.h"
#include "Node.h"

struct UIEvent {
    Node* targetNode;
    EventType type;
    EventPayload payload;
    
    // Default ctor
    UIEvent() : targetNode(nullptr), type(EventType::None), payload(EventType::None) {}

    UIEvent(EventType eventType, Node* node) : targetNode(node), type(eventType), payload(eventType) {
		
	}
};