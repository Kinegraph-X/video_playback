#pragma once
#include "player_headers.h"
#include "Node.h"

struct UIEvent {
    Node* targetNode;
    EventType type;
    EventPayload payload;
};