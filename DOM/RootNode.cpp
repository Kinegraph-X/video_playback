#include "DOM/RootNode.h"

RootNode::RootNode() : Node(nullptr) {}

// Override getParent to return nullptr
Node* RootNode::getParent() const { return nullptr; }
