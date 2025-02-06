#include "InteractionHandler.h"

InteractionHandler::InteractionHandler(std::shared_ptr<EventQueue> queue) : eventQueue(queue){
	
}
    
InteractionHandler::~InteractionHandler() {

}

void InteractionHandler::acquireRenderableNodes(std::vector<RenderableNode*>* nodes) {
	renderableNodes = nodes;
}

RaylibVector2 InteractionHandler::GetMousePositionScreen() {
    POINT point;
    GetCursorPos(&point); // Retrieves absolute screen position
    return {static_cast<float>(point.x), static_cast<float>(point.y)};
}

Node* InteractionHandler::findTargetNode(const RaylibVector2& position, bool isOutsideTheWindow) {
//	logger(LogLevel::DEBUG, "Testing mouse position " + LogUtils::toString(position));
//	logger(LogLevel::DEBUG, "isOutsideTheWindow " + LogUtils::toString(isOutsideTheWindow));
    for (auto it = renderableNodes->rbegin(); it != renderableNodes->rend(); it++) {
        if ((*it)->node->computedStyle.isVisible.value) {
			
			if (RaylibCheckCollisionPointRec(position, (*it)->bounds)) {
//				logger(LogLevel::DEBUG, "FOUND NODE ON EVENT");
//				if ((*it)->node->classNames.size() > 0) {
//					logger(LogLevel::DEBUG, "found node of class " + (*it)->node->classNames.at(0));
//				}
//				logger(LogLevel::DEBUG, "found node of class " + LogUtils::toString((*it)->bounds));
	            return (*it)->node;
			}
			else if ((*it)->node->isHovered) {
				(*it)->node->handleMouseOut(position);
			}
			else if (isOutsideTheWindow) {
				(*it)->node->handleMouseOut(position, true);
			}
        }
    }
    return nullptr;
}

void InteractionHandler::handleMouseEvents() {
    RaylibVector2 mousePosition = RaylibGetMousePosition();
    RaylibVector2 screenMousePosition = GetMousePositionScreen();
    bool isOutsideTheWindow = mousePosition == RaylibVector2{0, 0};
    Node* targetNode = findTargetNode(mousePosition, isOutsideTheWindow);

    // Mouse move
    if (targetNode) {
        EventPayload payload{EventType::MouseMove};
        payload.mousePosition = mousePosition;
        payload.screenMousePosition = screenMousePosition;
        UIEvent event(EventType::MouseMove, targetNode);
        event.payload = payload;
        eventQueue->pushEvent(event);
    }

    // Mouse buttons
    for (int button = 0; button < 3; button++) {
        if (RaylibIsMouseButtonPressed(button)) {
//			logger(LogLevel::DEBUG, "HANDLING MOUSEDOWN");
            if (targetNode) {
                EventPayload payload{EventType::MouseDown};
                payload.mousePosition = mousePosition;
                payload.screenMousePosition = screenMousePosition;
                payload.mouseButton = button;
                UIEvent event(EventType::MouseDown, targetNode);
        		event.payload = payload;
                eventQueue->pushEvent(event);
//				logger(LogLevel::DEBUG, "FOUND NODE ON MOUSEDOWN");
//				if (targetNode->classNames.size() > 0) {
//					logger(LogLevel::DEBUG, "found node of class " + targetNode->classNames.at(0));
//					logger(LogLevel::DEBUG, "found node of class " + LogUtils::toString(targetNode->computedStyle.bounds.value));
//				}
//                targetNode->handleEvent(payload);
            }
        }
        if (RaylibIsMouseButtonReleased(button)) {
//			logger(LogLevel::DEBUG, "HANDLING MOUSEUP");
            if (targetNode) {
                EventPayload payload{EventType::MouseUp};
                payload.mousePosition = mousePosition;
                payload.screenMousePosition = screenMousePosition;
                payload.mouseButton = button;
                UIEvent event(EventType::MouseUp, targetNode);
        		event.payload = payload;
                eventQueue->pushEvent(event);
                
//                targetNode->handleEvent(payload);
            }
        }
    }

    // Mouse wheel
    float wheelMove = RaylibGetMouseWheelMove();
    if (wheelMove != 0 && targetNode) {
//		logger(LogLevel::DEBUG, "HANDLING MOUSEWHEEL");
        EventPayload payload{EventType::MouseWheel};
        payload.mousePosition = mousePosition;
        payload.wheelMove = wheelMove;
        UIEvent event(EventType::MouseWheel, targetNode);
		event.payload = payload;
        eventQueue->pushEvent(event);
    }
}

void InteractionHandler::handleKeyboardEvents() {
	// We've not yet implemented the handling of focus
	// For now we'll handle the keyboard in a fake node
	Node* targetNode = nullptr; 
	
    // Key pressed
    int key = RaylibGetKeyPressed();
    if (key != 0) {
        EventPayload payload{EventType::KeyDown};
        payload.keyCode = key;
        UIEvent event(EventType::KeyDown, targetNode);
		event.payload = payload;
        eventQueue->pushEvent(event);
    }

    // Key released
    for (int key = 21; key < 255; key++) {
	    bool released = RaylibIsKeyReleased(key);
	    if (released != 0) {
	        EventPayload payload{EventType::KeyUp};
	        payload.keyCode = key;
	        UIEvent event(EventType::KeyUp, targetNode);
    		event.payload = payload;
	        eventQueue->pushEvent(event);
	    }
	}

    // Char input
//    int charPressed = RaylibGetCharPressed();
//    if (charPressed != 0) {
//        EventPayload payload{EventType::CharInput};
//        payload.charCode = charPressed;
//        UIEvent event(EventType::CharInput, targetNode);
//		event.payload = payload;
//        eventQueue->pushEvent(event);
//    }
}

void InteractionHandler::handleShouldClose() {
	if (RaylibWindowShouldClose()) {
		EventPayload payload{EventType::Close};
        UIEvent event(EventType::Close, renderableNodes->at(0)->node);
		event.payload = payload;
        eventQueue->pushEvent(event);
	}
}

void InteractionHandler::consumeEvents() {
	handleShouldClose();
    handleMouseEvents();
//    handleKeyboardEvents();
}

void InteractionHandler::handlerLoop() {
    while (running.load(std::memory_order_acquire)) {
//		RaylibPollInputEvents();
		consumeEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 fps
    }
    logger(LogLevel::DEBUG, "InteractionHandler::cleanup exited its infinite loop");
}

void InteractionHandler::start() {
    handlerThread = std::thread(&InteractionHandler::handlerLoop, this);
}

void InteractionHandler::stop() {
	logger(LogLevel::DEBUG, "InteractionHandler::cleanup START of exit sequence");
    running.store(false, std::memory_order_release);
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
    if (handlerThread.joinable()) {
        handlerThread.join();
    }
    logger(LogLevel::DEBUG, "InteractionHandler::cleanup END of exit sequence");
}


