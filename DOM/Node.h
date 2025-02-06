#pragma once
#include "DOM/EventListener.h"
#include "DOM/Style.h"
#include "ComputedStyle.h"

class Node : public EventListener {
protected:
	std::mutex nodeMutex;
	
private:
    void initNodeEventTypes();
    
//    void bubbleEvent(const EventPayload& payload);

protected:

void handleMouseEnter(const EventPayload& payload);

public:
	std::string id = "";
	std::vector<std::string> classNames;
	Node* parent;
    std::vector<Node*> children;
    Style* style = new Style();
//    Texture2D texture;  // Uninitialized Texture2D
    bool textureInitialized = false;
    std::string textContent;
    ComputedStyle computedStyle = ComputedStyle();
    
    std::atomic<bool> isActive{false};
    std::atomic<bool> isEntered{false};
    std::atomic<bool> isHovered{false};
    std::atomic<bool> isDragging{false};
    
    Node(Node* parent = nullptr, std::string id = "", std::vector<std::string> classNames = std::vector<std::string>());
    virtual ~Node();
	
	std::string getId();
	std::vector<std::string> getClassNames();

    void setParent(Node* newParent);
    Node* getParent();

    void addChild(Node* child);
    
    void removeChild(Node* child);
    
    std::vector<Node*> getChildren();
    
    virtual void handleEvent(const EventPayload& payload) override;
    /**
    * The main loop must check the isHovered status and call this only if appropriate
    */
    void handleMouseOut(RaylibVector2 mousePosition, bool isCleanupForOutsideWindow = false);
    void toggleActive();
	void setHovered(bool hovered);
    
    void setStyle(const Style& newStyle);
    Style& getStyle();
    ComputedStyle& getComputedStyle();
    
    void updateComputedStyle();
    void applyInheritedStyles();
	
	void setTextContent(const std::string text);
    std::string getTextContent();


//    void setTexture(const Texture2D& newTexture);
    void setBackgroundColor(RaylibColor color);
    void setBorderColor(RaylibColor color);
    void setTextColor(RaylibColor color);
    void setBackgroundImage(std::string name);
    
    void dispatchEvent(const EventPayload& payload);
    
};
