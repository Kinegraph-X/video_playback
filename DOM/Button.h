#pragma once
#include "DOM/Clickable.h"

class Button : public Clickable {
public:
    Button(Node* parent = nullptr, std::string id = "", std::vector<std::string> className = std::vector<std::string>());

    void onPress() override;

    void onRelease() override;
};
