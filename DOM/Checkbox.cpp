#pragma once
#include "Checkbox.h"

Checkbox::Checkbox(Node* parent, std::string id, std::vector<std::string> className) : Clickable(parent), isChecked(false) {}

void Checkbox::onPress() {}

void Checkbox::onRelease() {
    isChecked.store(!isChecked);
    updateAppearance();
}

void Checkbox::updateAppearance() {
    if (isChecked.load()) {
        setTextContent("☑");  // Checked
    } else {
        setTextContent("☐");  // Unchecked
    }
}

bool Checkbox::getChecked() const { return isChecked.load(); }
