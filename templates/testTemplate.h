#include "../player_headers.h"
#include "../DOM/NodeCreator.h"
#include "styleDefinitions.h"
using namespace pugi;

char templateName = "testTemplate.xml"
xml_document doc;
xml_parse_result result = doc.load_file(templateName);
if (!result) {
	logger(LogLevel::ERR, "Failed to load  a template : " + std::string(templateName))
}

NodeCreatorWalker walker(styleManager);
doc.traverse(walker);

Node* rootNode = walker.currentParent;