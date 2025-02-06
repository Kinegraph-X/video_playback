#pragma once
#include "Clickable.h"


Clickable::Clickable(Node* parent, std::string id, std::vector<std::string> classNames) 
		: Node(parent, id, classNames) {
    initEventType(EventType::MouseDown);
    initEventType(EventType::MouseMove);
    initEventType(EventType::MouseUp);
}

void Clickable::onPress() {}
void Clickable::onRelease() {}
void Clickable::setOnDragStart(DragCallback callback) { onDragStart = std::move(callback); }
void Clickable::setOnDragMove(DragCallback callback) { onDragMove = std::move(callback); }
void Clickable::setOnDragEnd(DragCallback callback) { onDragEnd = std::move(callback); }

void Clickable::handleHover(const EventPayload& payload) {
	// resetting the cursor to "arrow" is handled by the Node type
	RaylibSetMouseCursor(RAYLIB_MOUSE_CURSOR_POINTING_HAND);
}

void Clickable::handleClick(const EventPayload& payload) {
    switch (payload.type) {
        case EventType::MouseDown:
            {
//                std::lock_guard<std::mutex> lock(dragMutex);
                dragStartPosition = payload.screenMousePosition;
                lastDragPosition = payload.screenMousePosition;
//                logger(LogLevel::DEBUG, "Mouse move dragStart is " + LogUtils::toString(dragStartPosition));
//                logger(LogLevel::DEBUG, "Mouse move lastDrag is " + LogUtils::toString(lastDragPosition));
            }
//            onPress();
            break;

        case EventType::MouseUp:
        	if (onDragEnd) {
               onDragEnd(payload.screenMousePosition);
			}
			
            isDragging.store(false);
            
            {
	            EventPayload forwardedPayloadMouseUp{
					EventType::DragEnd
				};
				forwardedPayloadMouseUp.mousePosition = payload.mousePosition; 
	            EventListener::handleEvent(forwardedPayloadMouseUp);
            }
//            onRelease();
            break;

        case EventType::MouseMove:
//        	logger(LogLevel::DEBUG, "Mouse move init");
            if (isActive.load()) {
//				logger(LogLevel::DEBUG, "Mouse move isActive");
//				std::lock_guard<std::mutex> lock(dragMutex);
                if (!isDragging.load()) {
					
                    // Check if the mouse has moved enough to start dragging
                    if (RaylibVector2Distance(payload.screenMousePosition, dragStartPosition) > 2.0f) {
						logger(LogLevel::DEBUG, "Mouse move tested... isDragging is " + LogUtils::toString(isDragging));
                        isDragging = true;
                        if (onDragStart) {
                        	onDragStart(dragStartPosition);
						}
						
                        EventPayload forwardedPayload{
							EventType::DragStart
						};
						forwardedPayload.mousePosition = payload.mousePosition; 
                        EventListener::handleEvent(forwardedPayload);
                    }
                }
                else {
//					logger(LogLevel::DEBUG, "Mouse move : position is " + LogUtils::toString(payload.screenMousePosition));
//                	logger(LogLevel::DEBUG, "Mouse move lastDrag is " + LogUtils::toString(lastDragPosition));
					RaylibVector2 dragVector = RaylibVector2Subtract(payload.screenMousePosition, lastDragPosition);
					lastDragPosition = payload.screenMousePosition;
					if (onDragMove) {
                    	onDragMove(dragVector);
                    }
                    
                    EventPayload forwardedPayload{
						EventType::DragMove
					};
					forwardedPayload.mousePosition = payload.mousePosition; 
                    EventListener::handleEvent(forwardedPayload);
                }
            }
            break;
    }
}


void Clickable::handleEvent(const EventPayload& payload) {
    if (payload.type == EventType::MouseDown) {
        toggleActive();
        handleClick(payload);
        logger(LogLevel::DEBUG, "CLICKABLE HANDLE EVENT CALLED. Active state is " + LogUtils::toString(isActive));
//        onPress();
    } else if (payload.type == EventType::MouseUp) {
        toggleActive();
        handleClick(payload);
//        logger(LogLevel::DEBUG, "CLICKABLE HANDLE EVENT CALLED. Active state is " + LogUtils::toString(isActive));
//        onRelease();
    }
    else if (payload.type == EventType::MouseMove) {
		// resetting the cursor to "arrow" is handled by the Node type
		if (!isHovered.load()) {
        	handleHover(payload);
        }
        handleClick(payload);
//        logger(LogLevel::DEBUG, "CLICKABLE HANDLE EVENT CALLED. Active state is " + LogUtils::toString(isActive));
    }
    
    Node::handleEvent(payload);
}

RaylibVector2 Clickable::getDragDelta() {
	std::lock_guard<std::mutex> lock(dragMutex);
    return RaylibVector2Subtract(lastDragPosition, dragStartPosition);
}

