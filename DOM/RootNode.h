#pragma once
#include "DOM/Node.h"

class RootNode : public Node {
public:
    RootNode();
    
    // Override getParent to return nullptr
    Node* parent = nullptr;
    Node* getParent() const;
};