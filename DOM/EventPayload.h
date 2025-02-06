#pragma once
#include "player_headers.h"
#include "DOM/EventType.h"

struct EventPayload {
    EventType type;
    // Common fields
    RaylibVector2 mousePosition;
    RaylibVector2 screenMousePosition;
    int mouseButton;
    float wheelMove;
    int keyCode;
    int charCode;
    // Add more fields as needed

    // Constructor for different event types
    EventPayload(EventType t) : type(t) {}
};
