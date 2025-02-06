#pragma once
#include "player_headers.h"
#include "DOM/BasicLayout.h"
#include "DOM/EventQueue.h"

class InteractionHandler {
private:
    std::thread handlerThread;
    std::atomic<bool> running{true};
    std::shared_ptr<EventQueue> eventQueue;

    Node* findTargetNode(const RaylibVector2& position, bool isOutsideTheWindow);

    void handlerLoop();

public:
	std::vector<RenderableNode*>* renderableNodes;
    InteractionHandler(std::shared_ptr<EventQueue> queue);

    void start();
    void stop();
    void acquireRenderableNodes(std::vector<RenderableNode*>* renderableNodes);
    RaylibVector2 GetMousePositionScreen();
    
    void consumeEvents();
    void handleShouldClose();
	void handleMouseEvents();
    void handleKeyboardEvents();

    ~InteractionHandler();
};
