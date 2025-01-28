#pragma once
#include "player_headers.h"
#include "DOM/BasicLayout.h"
#include "DOM/EventQueue.h"

class InteractionHandler {
private:
    std::thread handlerThread;
    std::atomic<bool> running{true};
    EventQueue& eventQueue;
    std::vector<RenderableNode>* renderableNodes;

    Node* findTargetNode(const RaylibVector2& position);

    void handleMouseEvents();

    void handleKeyboardEvents();

    void handlerLoop();

public:
    InteractionHandler(EventQueue& queue);

    void start();
    void stop();
    void acquireRenderableNodes(std::vector<RenderableNode>* renderableNodes);

    ~InteractionHandler();
};
