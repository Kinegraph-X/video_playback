#pragma once
#include "DOM/Node.h"

class RootNode : public Node {
public:
    RootNode(Node* parent = nullptr, std::string id = "", std::vector<std::string> classNames = std::vector<std::string>());
    
    Node* getParent() const;
};