#pragma once
#include "DOM/Clickable.h"

class Checkbox : public Clickable {
private:
    std::atomic<bool> isChecked{false};
    std::mutex checkboxMutex;

public:
    Checkbox(Node* parent = nullptr, std::string id = "", std::vector<std::string> className = std::vector<std::string>());
	
	void onPress() override;
    void onRelease() override;

    void updateAppearance();

    bool getChecked() const;
};
