#include "InteractionHandler.h"

InteractionHandler::InteractionHandler(EventQueue& queue) : eventQueue(queue){
	
}
    
InteractionHandler::~InteractionHandler() {
    stop();
}

void InteractionHandler::acquireRenderableNodes(std::vector<RenderableNode*>* nodes) {
	renderableNodes = nodes;
}

Node* InteractionHandler::findTargetNode(const RaylibVector2& position) {
    for (auto it : *renderableNodes) {
        if (RaylibCheckCollisionPointRec(position, it->node->style->bounds.value)) {
            return it->node;
        }
    }
    return nullptr;
}

void InteractionHandler::handleMouseEvents() {
    RaylibVector2 mousePosition = RaylibGetMousePosition();
    Node* targetNode = findTargetNode(mousePosition);

    // Mouse move
    if (targetNode) {
        EventPayload payload{EventType::MouseMove};
        payload.mousePosition = mousePosition;
        UIEvent event(EventType::MouseMove, targetNode);
        event.payload = payload;
        eventQueue.pushEvent(event);
    }

    // Mouse buttons
    for (int button = 0; button < 3; button++) {
        if (RaylibIsMouseButtonPressed(button)) {
            if (targetNode) {
                EventPayload payload{EventType::MouseDown};
                payload.mousePosition = mousePosition;
                payload.mouseButton = button;
                UIEvent event(EventType::MouseMove, targetNode);
        		event.payload = payload;
                eventQueue.pushEvent(event);
            }
        }
        if (RaylibIsMouseButtonReleased(button)) {
            if (targetNode) {
                EventPayload payload{EventType::MouseUp};
                payload.mousePosition = mousePosition;
                payload.mouseButton = button;
                UIEvent event(EventType::MouseMove, targetNode);
        		event.payload = payload;
                eventQueue.pushEvent(event);
            }
        }
    }

    // Mouse wheel
    float wheelMove = RaylibGetMouseWheelMove();
    if (wheelMove != 0 && targetNode) {
        EventPayload payload{EventType::MouseWheel};
        payload.mousePosition = mousePosition;
        payload.wheelMove = wheelMove;
        UIEvent event(EventType::MouseMove, targetNode);
		event.payload = payload;
        eventQueue.pushEvent(event);
    }
}

void InteractionHandler::handleKeyboardEvents() {
	// We've not yet implemented the handling of focus
	// For now we'll handle the keyboard in a fake node
	Node* targetNode; 
	
    // Key pressed
    int key = RaylibGetKeyPressed();
    if (key != 0) {
        EventPayload payload{EventType::KeyDown};
        payload.keyCode = key;
        UIEvent event(EventType::MouseMove, targetNode);
		event.payload = payload;
        eventQueue.pushEvent(event);
    }

    // Key released
    for (int key = 21; key < 255; key++) {
	    bool released = RaylibIsKeyReleased(key);
	    if (released != 0) {
	        EventPayload payload{EventType::KeyUp};
	        payload.keyCode = key;
	        UIEvent event(EventType::MouseMove, targetNode);
    		event.payload = payload;
	        eventQueue.pushEvent(event);
	    }
	}

    // Char input
    int charPressed = RaylibGetCharPressed();
    if (charPressed != 0) {
        EventPayload payload{EventType::CharInput};
        payload.charCode = charPressed;
        UIEvent event(EventType::MouseMove, targetNode);
		event.payload = payload;
        eventQueue.pushEvent(event);
    }
}

void InteractionHandler::handlerLoop() {
    while (running) {
        handleMouseEvents();
        handleKeyboardEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 fps
    }
}

void InteractionHandler::start() {
    handlerThread = std::thread(&InteractionHandler::handlerLoop, this);
}

void InteractionHandler::stop() {
    running = false;
    if (handlerThread.joinable()) {
        handlerThread.join();
    }
}


