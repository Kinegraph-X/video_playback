#pragma once

struct EventPayload {
    EventType type;
    // Common fields
    Vector2 mousePosition;
    int keyCode;
    // Add more fields as needed

    // Constructor for different event types
    EventPayload(EventType t) : type(t) {}
};
