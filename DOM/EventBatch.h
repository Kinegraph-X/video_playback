#pragma once
#include "player_headers.h"
#include "DOM/Node.h"
#include "DOM/UIEvent.h"

class EventBatch {
private:
    std::unordered_map<Node*, std::unordered_map<EventType, UIEvent>> events;

public:
	EventBatch();
	~EventBatch();
	
    void addEvent(const UIEvent& event);

    std::vector<UIEvent> getEvents();
    
    void clear();
    void cleanup();
};