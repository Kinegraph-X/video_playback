#pragma once
#include "Checkbox.h"

Checkbox::Checkbox(Node* parent = nullptr) : Clickable(parent), isChecked(false) {}

void Checkbox::onRelease() override {
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
