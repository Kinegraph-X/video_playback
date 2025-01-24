#pragma once
#include <atomic>
#include <mutex>
#include "EventListener"
#include "Style.h"



Node::Node(Node* parent = nullptr, char* id = nullptr, char* className = nullptr) : parent(parent) {
    if (parent) {
        parent->addChild(this);
    }
    // Initialize common event types
}
Node::~Node() {
    if (textureInitialized) {
        UnloadTexture(texture);
    }
    if (style) {
        delete style;
        style = nullptr;
    }
}


void Node::setParent(Node* newParent) {
	std::lock_guard<std::mutex> lock(nodeMutex);
    if (parent) {
        parent->removeChild(this);
    }
    parent = newParent;
    if (parent) {
        parent->addChild(std::unique_ptr<Node>(this));
    }
}

void Node::addChild(std::unique_ptr<Node> child) {
	std::lock_guard<std::mutex> lock(nodeMutex);
    child->parent = this;
    children.push_back(std::move(child));
}

void Node::removeChild(Node* child) {
	std::lock_guard<std::mutex> lock(nodeMutex);
    auto it = std::find_if(children.begin(), children.end(),
                           [child](const std::unique_ptr<Node>& c) { return c.get() == child; });
    if (it != children.end()) {
        (*it)->parent = nullptr;
        children.erase(it);
    }
}

void Node::void setStyle(const Style& newStyle) {
	std::lock_guard<std::mutex> lock(nodeMutex);
	if (style) {
        delete style;
    }
    style = new Style(newStyle);  // Create a copy of newStyle on the heap
}


Style& Node::getStyle() {
	std::lock_guard<std::mutex> lock(nodeMutex);
	return style;
}

void Node::setTextContent(const std::string& text) {
    std::lock_guard<std::mutex> lock(nodeMutex);
    textContent = text;
}

std::string Node::getTextContent() const {
    std::lock_guard<std::mutex> lock(nodeMutex);
    return textContent;
}


// Texture management
void Node::setTexture(const Texture2D& newTexture) {
	std::lock_guard<std::mutex> lock(nodeMutex);
    if (textureInitialized) {
        UnloadTexture(texture);
    }
    texture = newTexture;
    textureInitialized = true;
}

void Node::initNodeEventTypes() {
    // Initialize additional event types specific to nodes if needed
}

void Node::dispatchEvent(Node* root, Node* target, const EventPayload& payload) {
    std::vector<Node*> path;
    Node* current = target;
    
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
    target->handleEvent(payload);
    
    // Bubbling phase
    for (auto it = path.rbegin(); it >= path.rend(); ++it) {
        (*it)->handleEvent(payload);
    }
}

void Node::bubbleEvent(const EventPayload& payload) {
    dispatchEvent(payload);
//	if (parent) {
//      parent->bubbleEvent(payload);
//   }
}
