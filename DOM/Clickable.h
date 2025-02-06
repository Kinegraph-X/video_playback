#pragma once
#include "DOM/Node.h"

class Clickable : public Node {
public:
    Clickable(Node* parent = nullptr, std::string id = "", std::vector<std::string> classNames = std::vector<std::string>());

    virtual void onPress();
    virtual void onRelease();
    
    using DragCallback = std::function<void(const RaylibVector2&)>;

    void setOnDragStart(DragCallback callback);
    void setOnDragMove(DragCallback callback);
    void setOnDragEnd(DragCallback callback);
    
    DragCallback onDragStart;
    DragCallback onDragMove;
    DragCallback onDragEnd;

    
protected:
    RaylibVector2 dragStartPosition;
    RaylibVector2 lastDragPosition;
	std::mutex dragMutex;
	
	void handleHover(const EventPayload& payload);
    void handleClick(const EventPayload& payload);

    virtual void handleEvent(const EventPayload& payload) override;
    
	RaylibVector2 getDragDelta();

};
