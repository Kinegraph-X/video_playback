#pragma once
#include "Clickable.h"

class Checkbox : public Clickable {
private:
    std::atomic<bool> isChecked(false);
    std::mutex checkboxMutex;

public:
    Checkbox(Node* parent = nullptr);

    void onRelease() override;

    void updateAppearance();

    bool getChecked() const;
};
