#include "StyleManager.h"

void StyleManager::addIdStyle(const std::string& id, const Style& style) {
    idStyles[id] = style;
}

void StyleManager::addClassStyle(const std::string& className, const Style& style) {
    classStyles[className] = style;
}

const Style& StyleManager::getStyle(const std::string& id, const std::string& className) const {
    auto idIt = idStyles.find(id);
    if (idIt != idStyles.end()) {
        return idIt->second;
    }

    auto classIt = classStyles.find(className);
    if (classIt != classStyles.end()) {
        return classIt->second;
    }

    return defaultStyle;
}
