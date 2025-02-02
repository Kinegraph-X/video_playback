#include "DOM/RootNode.h"

RootNode::RootNode() {}

// Override getParent to return nullptr
Node* RootNode::getParent() const { return nullptr; }
