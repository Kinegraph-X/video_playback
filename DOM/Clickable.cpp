#pragma once
#include "Node.h"


Clickable::Clickable(Node* parent = nullptr) : Node(parent), isPressed(false) {
    initEventType(EventType::MouseDown);
    initEventType(EventType::MouseMove);
    initEventType(EventType::MouseUp);
}

void Clickable::toggleActive() {
    isActive = !isActive;
    updateAppearance();
}

void Clickable::updateAppearance() {
    Style& style = getStyle();
    if (isActive) {
        setBackgroundColor(style.activeBackgroundColor);
        setBorderColor(style.activeBorderColor);
        setTextColor(style.activeTextColor);
        setBackgroundImage(style.activeBackgroundImage);
    } else {
        setBackgroundColor(style.backgroundColor);
        setBorderColor(style.borderColor);
        setTextColor(style.textColor);
        setBackgroundImage(style.backgroundImage);
    }
}

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
                    onDragEnd();
                    isDragging.store(false);
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
                    if (Vector2Distance(dragStartPosition, payload.mousePosition) > 5.0f) {
                        isDragging = true;
                        onDragStart();
                    }
                }
                if (isDragging.load()) {
                    onDragMove();
                    lastDragPosition = payload.mousePosition;
                }
            }
            break;
    }
}


void Clickable::handleEvent(const EventPayload& payload) override {
    if (payload.type == EventType::MouseDown) {
        toggleActive();
        onPress();
    } else if (payload.type == EventType::MouseUp) {
        toggleActive();
        onRelease();
    }
    handleClick(payload);
    Node::handleEvent(payload);
}

Vector2 Clickable::getDragDelta() const {
	std::lock_guard<std::mutex> lock(dragMutex);
    return Vector2Subtract(lastDragPosition, dragStartPosition);
}

