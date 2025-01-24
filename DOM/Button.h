#pragma once
#include "Clickable.h"

class Button : public Clickable {
public:
    Button(Node* parent = nullptr);

    void onPress() override;

    void onRelease() override;
};
