#pragma once
#include "DOM/Clickable.h"

class Checkbox : public Clickable {
private:
    std::atomic<bool> isChecked{false};
    std::mutex checkboxMutex;

public:
    Checkbox(Node* parent = nullptr, char* id = nullptr, char* className = nullptr);
	
	void onPress() override;
    void onRelease() override;

    void updateAppearance();

    bool getChecked() const;
};
