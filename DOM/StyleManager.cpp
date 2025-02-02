#include "StyleManager.h"

void StyleManager::addIdStyle(const std::string& id, const Style& style) {
//	logger(LogLevel::DEBUG, "adding style for ID " + id);
    idStyles[id] = style;
}

void StyleManager::addClassStyle(const std::string& className, const Style& style) {
    classStyles[className] = style;
}

const Style& StyleManager::getStyle(const std::string& id, const std::string& className) const {
//	logger(LogLevel::DEBUG, "searching style for ID " + id);
//	logger(LogLevel::DEBUG, "Found style for ID " + LogUtils::toString(idStyles.count(id)));
    auto idIt = idStyles.find(id);
    if (idIt != idStyles.end()) {
//		logger(LogLevel::DEBUG, "Found style for ID " + id);
        return idIt->second;
    }

    auto classIt = classStyles.find(className);
    if (classIt != classStyles.end()) {
//        logger(LogLevel::DEBUG, "Found style for className " + className);
        return classIt->second;
    }

    return defaultStyle;
}

std::vector<std::string> StyleManager::getAllBackgroundImageFilenames() {
    std::unordered_set<std::string> uniqueFilenames;

    auto addFilenameIfSet = [&uniqueFilenames](const Style& style) {
        if (!style.backgroundImage.value.empty()) {
            uniqueFilenames.insert(style.backgroundImage.value);
        }
    };

    for (const auto& [id, style] : idStyles) {
        addFilenameIfSet(style);
    }
    for (const auto& [className, style] : classStyles) {
        addFilenameIfSet(style);
    }

    std::vector<std::string> result(uniqueFilenames.begin(), uniqueFilenames.end());
    
    return result;
}

void StyleManager::applyStylesToNode(Node* node) {
        // Apply id styles
        if (idStyles.count(node->getId()) > 0) {
            node->setStyle(idStyles[node->getId()]);
        }

        // Apply class styles
        for (const auto& className : node->getClassNames()) {
            if (classStyles.count(className) > 0) {
                node->setStyle(classStyles[className]);
            }
        }

        // Update computed style
        node->updateComputedStyle();

        // Recursively apply to children
        for (auto* child : node->getChildren()) {
            applyStylesToNode(child);
        }
    }


