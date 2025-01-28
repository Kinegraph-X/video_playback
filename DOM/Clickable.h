#pragma once
#include "DOM/Node.h"

class Clickable : public Node {
public:
    Clickable(Node* parent = nullptr, char* id = nullptr, char* className = nullptr);

    virtual void onPress();
    virtual void onRelease();
    
    using DragCallback = std::function<void(const RaylibVector2&)>;

    void setOnDragStart(DragCallback callback);
    void setOnDragMove(DragCallback callback);
    void setOnDragEnd(DragCallback callback);

    
    void setStyle(const Style& newStyle);

	void toggleActive();

    void updateAppearance();
    
    void handleClick(const EventPayload& payload);

    void handleEvent(const EventPayload& payload);
    
protected:
    RaylibVector2 dragStartPosition;
    RaylibVector2 lastDragPosition;
	std::atomic<bool> isActive{false};
    std::atomic<bool> isDragging{false};
	std::mutex dragMutex;
	Style* cachedStyle;
	
	RaylibVector2 getDragDelta();
	DragCallback onDragStart;
    DragCallback onDragMove;
    DragCallback onDragEnd;
};
