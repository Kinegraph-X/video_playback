#pragma once
#include "player_headers.h"
#include "DOM/UIEvent.h"
#include "DOM/EventBatch.h"

class EventQueue {
private:
    std::queue<UIEvent> eventQueue;
    std::mutex queueMutex;

public:
    void pushEvent(const UIEvent& event);

    EventBatch createBatch();
    
    void populateBatch(EventBatch& batch);
};


