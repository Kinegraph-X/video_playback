#pragma once
#include "player_headers.h"
#include "DOM/NodeCreator.h"
#include "templates/styleDefinitions.h"


pugi::xml_document loadTemplate(char* templateName) {
	namespace fs = std::filesystem;
	pugi::xml_document doc;
	
	if (!fs::exists(templateName)) {
        logger(LogLevel::ERR, "Error: Template file not found: " + std::string(templateName));
        return doc;  // Exit with error code
    }
    
    pugi::xml_parse_result xmlLoadingResult = doc.load_file(templateName);
	if (!xmlLoadingResult) {
		logger(LogLevel::ERR, "Failed to load  a template : " + std::string(templateName));
		return doc;
	}
	return doc;
}

