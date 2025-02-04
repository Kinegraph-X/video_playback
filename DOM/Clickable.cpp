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

void Clickable::handleClick(const EventPayload& payload) {
    switch (payload.type) {
        case EventType::MouseDown:
            isActive.store(true);
            {
                std::lock_guard<std::mutex> lock(dragMutex);
                dragStartPosition = payload.mousePosition;
                lastDragPosition = payload.mousePosition;
            }
            onPress();
            break;

        case EventType::MouseUp:
            if (isActive.load()) {
                if (isDragging.load()) {
                    onDragEnd(payload.mousePosition);
                    isDragging.store(false);
                    EventPayload forwardedPayload{
						EventType::DragEnd
					};
					forwardedPayload.mousePosition = payload.mousePosition; 
                    EventListener::handleEvent(forwardedPayload);
                }
                onRelease();
            }
            isActive.store(false);
            break;

        case EventType::MouseMove:
            if (isActive.load()) {
				std::lock_guard<std::mutex> lock(dragMutex);
                if (!isDragging.load()) {
                    // Check if the mouse has moved enough to start dragging
                    if (RaylibVector2Distance(dragStartPosition, payload.mousePosition) > 5.0f) {
                        isDragging = true;
                        onDragStart(dragStartPosition);
                        EventPayload forwardedPayload{
							EventType::DragStart
						};
						forwardedPayload.mousePosition = dragStartPosition; 
                        EventListener::handleEvent(forwardedPayload);
                    }
                }
                if (isDragging.load()) {
					RaylibVector2 dragVector = RaylibVector2Subtract(dragStartPosition, payload.mousePosition);
                    onDragMove(dragVector);
                    lastDragPosition = payload.mousePosition;
                    EventPayload forwardedPayload{
						EventType::DragMove
					};
					forwardedPayload.mousePosition = dragVector; 
                    EventListener::handleEvent(forwardedPayload);
                }
            }
            break;
    }
}


void Clickable::handleEvent(const EventPayload& payload) {
    if (payload.type == EventType::MouseDown) {
        toggleActive();
//        logger(LogLevel::DEBUG, "CLICKABLE HANDLE EVENT CALLED. Active state is " + LogUtils::toString(isActive));
        onPress();
    } else if (payload.type == EventType::MouseUp) {
        toggleActive();
//        logger(LogLevel::DEBUG, "CLICKABLE HANDLE EVENT CALLED. Active state is " + LogUtils::toString(isActive));
        onRelease();
    }
    handleClick(payload);
    EventListener::handleEvent(payload);
}

RaylibVector2 Clickable::getDragDelta() {
	std::lock_guard<std::mutex> lock(dragMutex);
    return RaylibVector2Subtract(lastDragPosition, dragStartPosition);
}

