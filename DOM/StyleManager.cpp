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

std::vector<std::string> StyleManager::getAllBackgroundImageFilenames() {
        std::unordered_set<std::string> uniqueFilenames;

        // Helper function to add filename if backgroundImage is set
        auto addFilenameIfSet = [&uniqueFilenames](const Style& style) {
            if (!style.backgroundImage.empty()) {
                uniqueFilenames.insert(style.backgroundImage);
            }
        };

        // Check id styles
        for (const auto& [id, style] : idStyles) {
            addFilenameIfSet(style);
        }
        // Check class styles
        for (const auto& [className, style] : classStyles) {
            addFilenameIfSet(style);
        }

        // Convert set to vector
        std::vector<std::string> result(uniqueFilenames.begin(), uniqueFilenames.end());
        
        // Optional: Sort the result for consistent ordering
        std::sort(result.begin(), result.end());

        return result;
    }

