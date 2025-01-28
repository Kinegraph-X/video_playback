#pragma once
#include "DOM/Clickable.h"

class Button : public Clickable {
public:
    Button(Node* parent = nullptr, char* id = nullptr, char* className = nullptr);

    void onPress() override;

    void onRelease() override;
};
