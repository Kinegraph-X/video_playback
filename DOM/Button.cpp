#pragma once
#include "Button.h"

Button::Button(Node* parent, std::string id, std::vector<std::string> classNames)
	 : Clickable(parent, id, classNames) {}

void Button::onPress() {
    
}

void Button::onRelease() {
    
}
