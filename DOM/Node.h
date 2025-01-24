#pragma once
#include <atomic>
#include <mutex>
#include "EventListener"
#include "Style.h"

class Node : public EventListener {
private:
	std::mutex nodeMutex;
    Node* parent;
    std::vector<std::unique_ptr<Node>> children;
    Style* style;
    Texture2D texture;  // Uninitialized Texture2D
    bool textureInitialized;
    std::string textContent;

public:
    Node(Node* parent = nullptr, char* id = nullptr, char* className = nullptr) : parent(parent);
    ~Node();
	
	char* id = "";
	char* className = "";

    void setParent(Node* newParent);

    void addChild(std::unique_ptr<Node> child);
    
    void removeChild(Node* child);
    
    void setStyle(const Style& newStyle);

    Style& getStyle();
	
	void setTextContent(const std::string& text);
    
    std::string getTextContent() const;


    // Texture management
    void setTexture(const Texture2D& newTexture);
    
    void initNodeEventTypes();
    
    void dispatchEvent(Node* root, Node* target, const EventPayload& payload);

    void bubbleEvent(const EventPayload& payload);

    // Other methods...
};
