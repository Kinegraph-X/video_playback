#pragma once
#include "../player_headers.h"
#include "Style.h"

class StyleManager {
private:
    std::unordered_map<std::string, Style> idStyles;
    std::unordered_map<std::string, Style> classStyles;
    Style defaultStyle;

public:
    void addIdStyle(const std::string& id, const Style& style);

    void addClassStyle(const std::string& className, const Style& style);

    const Style& getStyle(const std::string& id, const std::string& className) const;
};
