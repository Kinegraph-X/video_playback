#pragma once
#include "Node.h"

// If we were using std::unique_ptr<Node>&& parent
// Call with auto node = std::make_unique<Node>(std::move(parentNode), "id", "className");
// Remember that after moving a unique_ptr, the original pointer becomes null,
// so ensure we're not using the moved pointer afterwards
Node::Node(Node* parent, std::string id, std::vector<std::string> classNames) 
		: parent(parent), id(id), classNames(classNames) {
    if (parent) {
        parent->addChild(this);
    }
//    logger(LogLevel::DEBUG, "node classNames length : " + std::to_string(this->classNames.size()));
}
Node::~Node() {
//	logger(LogLevel::DEBUG, "cleaning node id : " + this->id);
//	if (this->classNames.size() > 0) {
//		logger(LogLevel::DEBUG, "cleaning node className : " + this->classNames.at(0));
//	}
//	logger(LogLevel::DEBUG, "node has children : " + std::to_string(this->children.size()));
    if (textureInitialized) {
        RaylibUnloadTexture(texture);
    }
    if (style) {
        delete style;
        style = nullptr;
    }
    for (Node* child : children) {
		logger(LogLevel::DEBUG, "cleaning node child");
        delete child;  // Recursively delete all children
        child = nullptr;
        logger(LogLevel::DEBUG, "cleaned node child");
    }
    children.clear();
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
//	logger(LogLevel::DEBUG, "ADD CHILD CALLED");
//	logger(LogLevel::DEBUG, "node child count: " + std::to_string(children.size()));
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

std::string Node::getId() {
	return this->id;
}

std::vector<std::string> Node::getClassNames() {
	return this->classNames;
}

void Node::toggleActive() {
    isActive = !isActive;
//    logger(LogLevel::DEBUG, "active state toggled : " + LogUtils::toString(isActive));
    updateComputedStyle();
}

void Node::setHovered(bool hovered) {
    isHovered = hovered;
    updateComputedStyle();
}

void Node::setStyle(const Style& newStyle) {
	std::lock_guard<std::mutex> lock(nodeMutex);
	if (style) {
        delete style;
    }
    style = new Style(newStyle);  // Create a copy of newStyle on the heap
//    logger(LogLevel::DEBUG, "storing value for backgroungImage : " + style->backgroundImage.value);
}


Style& Node::getStyle() {
	std::lock_guard<std::mutex> lock(nodeMutex);
	return *style;
}

ComputedStyle& Node::getComputedStyle() {
	std::lock_guard<std::mutex> lock(nodeMutex);
	return computedStyle;
}

void Node::updateComputedStyle() {
	std::lock_guard<std::mutex> lock(nodeMutex);
	for (const auto& property : computedStyle) {
		
		std::visit([this](const auto& prop) {
			if (prop.get().name.empty()) {
				logger(LogLevel::ERR, "while accessing name of ComputedStyle property : " + prop.get().name);
				return;
			}
			const PropertyVariant propVariant = (*style)[prop.get().name]; 
			const auto* styleProp = std::get_if<std::reference_wrapper<std::decay_t<decltype(prop.get())>>>(&propVariant);
//			if (this->id != "")
//				logger(LogLevel::DEBUG, "updated value in computed style for node id : " + id + " and propName : " + prop.get().name + " with value : " + LogUtils::toString(styleProp->get().value));
			prop.get().setValue(styleProp->get().value);
		}, property);
	}
//	logger(LogLevel::DEBUG, "updated main values of computed style for node id : " + id);
	if (isHovered) {
        if (!style->hoverBounds.isDefault) computedStyle.bounds.setValue(style->hoverBounds.value);
        if (!style->hoverBackgroundColor.isDefault) computedStyle.backgroundColor.setValue(style->hoverBackgroundColor.value);
        if (!style->hoverBorderWidth.isDefault) computedStyle.borderWidth.setValue(style->hoverBorderWidth.value);
        if (!style->hoverBorderColor.isDefault) computedStyle.borderColor.setValue(style->hoverBorderColor.value);
        if (!style->hoverTextColor.isDefault) computedStyle.textColor.setValue(style->hoverTextColor.value);
        if (!style->hoverBackgroundImage.isDefault) computedStyle.backgroundImage.setValue(style->hoverBackgroundImage.value);
    }

    // Apply active styles if active (overrides hover styles)
    if (isActive) {
        if (!style->activeBounds.isDefault) computedStyle.bounds.setValue(style->activeBounds.value);
        if (!style->activeBackgroundColor.isDefault) computedStyle.backgroundColor.setValue(style->activeBackgroundColor.value);
        if (!style->activeBorderWidth.isDefault) computedStyle.borderWidth.setValue(style->activeBorderWidth.value);
        if (!style->activeBorderColor.isDefault) computedStyle.borderColor.setValue(style->activeBorderColor.value);
        if (!style->activeTextColor.isDefault) computedStyle.textColor.setValue(style->activeTextColor.value);
        if (!style->activeBackgroundImage.isDefault) computedStyle.backgroundImage.setValue(style->activeBackgroundImage.value);
    }
    
//    applyInheritedStyles();
}

void Node::applyInheritedStyles() {
	std::lock_guard<std::mutex> lock(nodeMutex);
	for (const auto& property : computedStyle) {
		std::visit([this](const auto& prop) {
	        if (std::find(INHERITED_PROPERTIES.begin(), INHERITED_PROPERTIES.end(), prop.get().name) != INHERITED_PROPERTIES.end()) {
                // We must ensure `prop` and `parentProp` are compatible, as they inherit from different templates
//				const auto& parentPropVariant = (*parent->style)[prop.name];
//				std::visit([this, &prop](const auto &parentProp) {
//  					using PropType = std::decay_t<decltype(prop)>;
//  					using ParentPropType = std::decay_t<decltype(parentProp)>;
////  					auto* propPtr = std::get_if<PropType>(&(*style)[prop.name]);
//
//  					if constexpr (std::is_same_v<PropType, ParentPropType>) {
////						if (propPtr) {
//    					prop.setValue(parentProp.value);
//  					
////						ParentPropType modifiedProp = *propPtr;  // Create a mutable copy
////				        modifiedProp.setValue(parentProp.value);
////				        (*style)[parentProp.name] = modifiedProp;
////				        }
//			        }
//				},
//				parentPropVariant);

				// Alternative
				const PropertyVariant propVariant = (*parent->style)[prop.get().name];
				const auto* parentProp = std::get_if<std::reference_wrapper<std::decay_t<decltype(prop.get())>>>(&propVariant);
				if (parentProp && prop.get().isDefault) {
                    prop.get().setValue(parentProp->get().value);
                }
			}
	    }, property);
	}
}

void Node::setTextContent(const std::string text) {
    std::lock_guard<std::mutex> lock(nodeMutex);
    textContent = text;
}

std::string Node::getTextContent() {
    std::lock_guard<std::mutex> lock(nodeMutex);
    return textContent;
}


// Texture management
//void Node::setTexture(const Texture2D& newTexture) {
//	std::lock_guard<std::mutex> lock(nodeMutex);
//    if (textureInitialized) {
//        RaylibUnloadTexture(texture);
//    }
//    texture = newTexture;
//    textureInitialized = true;
//}

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
