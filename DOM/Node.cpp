#pragma once
#include "Node.h"

// If we were using std::unique_ptr<Node>&& parent
// Call with auto node = std::make_unique<Node>(std::move(parentNode), "id", "className");
// Remember that after moving a unique_ptr, the original pointer becomes null,
// so ensure we're not using the moved pointer afterwards
Node::Node(Node* parent, char* id, char* className) : parent(std::move(parent)) {
    if (parent) {
        parent->addChild(this);
    }
    // Initialize common event types
}
Node::~Node() {
    if (textureInitialized) {
        RaylibUnloadTexture(texture);
    }
    if (style) {
        delete style;
        style = nullptr;
    }
}


Node* Node::getParent() {
	return parent;
}

void Node::setParent(Node* newParent) {
	std::lock_guard<std::mutex> lock(nodeMutex);
    if (parent) {
        parent->removeChild(this);
    }
    parent = newParent;
    if (parent) {
        parent->addChild(this);
    }
}

void Node::addChild(Node* child) {
	std::lock_guard<std::mutex> lock(nodeMutex);
    child->parent = this;
    children.push_back(child);
}

void Node::removeChild(Node* child) {
	std::lock_guard<std::mutex> lock(nodeMutex);
    auto it = std::find_if(
		children.begin(),
		children.end(),
        [child](const Node* c) { return c == child; }
    );
    if (it != children.end()) {
        (*it)->parent = nullptr;
        children.erase(it);
    }
}

std::vector<Node*> Node::getChildren() {
	return children;
}

void Node::setStyle(const Style& newStyle) {
	std::lock_guard<std::mutex> lock(nodeMutex);
	if (style) {
        delete style;
    }
    style = new Style(newStyle);  // Create a copy of newStyle on the heap
}


Style& Node::getStyle() {
	std::lock_guard<std::mutex> lock(nodeMutex);
	return *style;
}

void Node::setTextContent(const std::string& text) {
    std::lock_guard<std::mutex> lock(nodeMutex);
    textContent = text;
}

std::string Node::getTextContent() {
    std::lock_guard<std::mutex> lock(nodeMutex);
    return textContent;
}


// Texture management
void Node::setTexture(const Texture2D& newTexture) {
	std::lock_guard<std::mutex> lock(nodeMutex);
    if (textureInitialized) {
        RaylibUnloadTexture(texture);
    }
    texture = newTexture;
    textureInitialized = true;
}

void Node::initNodeEventTypes() {
    // Initialize additional event types specific to nodes if needed
}

void Node::dispatchEvent(const EventPayload& payload) {
    std::vector<Node*> path;
    Node* current = this;
    
    // Build path from target to root
    while (current) {
        path.push_back(current);
        current = current->getParent();
    }
	    
	// Capturing phase
//	    for (auto it = path.rbegin(); it != path.rend(); ++it) {
//	        (*it)->handleCapturingEvent(payload);
//	    }
    
    // Target phase
    this->handleEvent(payload);
    
    // Bubbling phase
    for (auto it = path.rbegin(); it >= path.rend(); ++it) {
        (*it)->handleEvent(payload);
    }
}

//void Node::bubbleEvent(const EventPayload& payload) {
//    handleEvent(payload);
//	if (parent) {
//      parent->bubbleEvent(payload);
//   }
//}

void Node::setBackgroundColor(RaylibColor color) {
	style->backgroundColor = color;
}
void Node::setBorderColor(RaylibColor color) {
	style->borderColor = color;
}
void Node::setTextColor(RaylibColor color) {
	style->textColor = color;
}
void Node::setBackgroundImage(std::string name) {
	style->backgroundImage = name;
}
