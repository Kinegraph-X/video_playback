#pragma once
#include "DOM/Clickable.h"

class Button : public Clickable {
public:
    Button(Node* parent = nullptr, std::string id = "", std::vector<std::string> classNames = std::vector<std::string>());

    virtual void onPress() override;

    virtual void onRelease() override;
};
