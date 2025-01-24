#pragma once
#include "Node.h"

class Clickable : public Node {
protected:
    Vector2 dragStartPosition;
    Vector2 lastDragPosition;
	std::atomic<bool> isActive{false};
    std::atomic<bool> isDragging{false};
	std::mutex dragMutex;


public:
    Clickable(Node* parent = nullptr) : Node(parent), isPressed(false)

    virtual void onPress();
    virtual void onRelease();
    virtual void onDragStart();
    virtual void onDragMove();
    virtual void onDragEnd();

	void toggleActive();

    void updateAppearance();
    
    void handleClick(const EventPayload& payload);


    void handleEvent(const EventPayload& payload) override;

};
