#pragma once
#include "DOM/EventListener.h"
#include "DOM/Style.h"

class Node : public EventListener {
protected:
	std::mutex nodeMutex;
	
private:
    void initNodeEventTypes();
    
//    void bubbleEvent(const EventPayload& payload);

public:
	Node* parent;
    std::vector<Node*> children;
    Style* style;
    Texture2D texture;  // Uninitialized Texture2D
    bool textureInitialized;
    std::string textContent;
    
    Node(Node* parent = nullptr, char* id = nullptr, char* className = nullptr);
    ~Node();
	
	char* id = "";
	char* className = "";

    void setParent(Node* newParent);
    Node* getParent();

    void addChild(Node* child);
    
    void removeChild(Node* child);
    
    std::vector<Node*> getChildren();
    
    void setStyle(const Style& newStyle);

    Style& getStyle();
	
	void setTextContent(const std::string& text);
    
    std::string getTextContent();


    void setTexture(const Texture2D& newTexture);
    void setBackgroundColor(RaylibColor color);
    void setBorderColor(RaylibColor color);
    void setTextColor(RaylibColor color);
    void setBackgroundImage(std::string name);
    
    void dispatchEvent(const EventPayload& payload);
    
};
