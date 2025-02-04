#pragma once
#include "player_headers.h"
#include "DOM/BasicLayout.h"
#include "DOM/EventQueue.h"

class InteractionHandler {
private:
    std::thread handlerThread;
    std::atomic<bool> running{true};
    EventQueue& eventQueue;

    Node* findTargetNode(const RaylibVector2& position);

    void handlerLoop();

public:
	std::vector<RenderableNode*>* renderableNodes;
    InteractionHandler(EventQueue& queue);

    void start();
    void stop();
    void acquireRenderableNodes(std::vector<RenderableNode*>* renderableNodes);
    
    void consumeEvents();
    void handleShouldClose();
	void handleMouseEvents();
    void handleKeyboardEvents();

    ~InteractionHandler();
};
