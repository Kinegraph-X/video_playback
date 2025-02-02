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

public:
	std::string id = "";
	std::vector<std::string> classNames;
	Node* parent;
    std::vector<Node*> children;
    Style* style = new Style();
    Texture2D texture;  // Uninitialized Texture2D
    bool textureInitialized;
    std::string textContent;
    ComputedStyle computedStyle = ComputedStyle();
    
    std::atomic<bool> isActive{false};
    std::atomic<bool> isHovered{false};
    std::atomic<bool> isDragging{false};
    
    Node(Node* parent = nullptr, std::string id = "", std::vector<std::string> className = std::vector<std::string>());
    ~Node();
	
	std::string getId();
	std::vector<std::string> getClassNames();

    void setParent(Node* newParent);
    Node* getParent();

    void addChild(Node* child);
    
    void removeChild(Node* child);
    
    std::vector<Node*> getChildren();
    
    void toggleActive();
	void setHovered(bool hovered);
    
    void setStyle(const Style& newStyle);
    Style& getStyle();
    ComputedStyle& getComputedStyle();
    
    void updateComputedStyle();
    void applyInheritedStyles();
	
	void setTextContent(const std::string text);
    std::string getTextContent();


    void setTexture(const Texture2D& newTexture);
    void setBackgroundColor(RaylibColor color);
    void setBorderColor(RaylibColor color);
    void setTextColor(RaylibColor color);
    void setBackgroundImage(std::string name);
    
    void dispatchEvent(const EventPayload& payload);
    
};
