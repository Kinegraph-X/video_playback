#include "DOM/RootNode.h"

RootNode::RootNode(Node* parent, std::string id, std::vector<std::string> classNames) {
	this->classNames.push_back("no_class");
//	logger(LogLevel::DEBUG, "root node classNames length : " + std::to_string(this->classNames.size()));
}

// Override getParent to return nullptr
Node* RootNode::getParent() const { return nullptr; }
