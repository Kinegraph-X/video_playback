#pragma once
#include  "player_headers.h"
#include "DOM/StyleManager.h"
#include "DOM/RootNode.h"
#include "DOM/Clickable.h"
#include "DOM/Button.h"
#include "DOM/Checkbox.h"

struct NodeCreatorWalker : pugi::xml_tree_walker {
    RootNode* rootNode = new RootNode();
    Node* currentParent = rootNode;
    const StyleManager& styleManager;

    NodeCreatorWalker(const StyleManager& sm) : styleManager(sm) {
		rootNode->id = "root_node";
//		logger(LogLevel::DEBUG, "NodeCreatorWalker ctor");
	}

    virtual bool for_each(pugi::xml_node& xmlNode) {
//		logger(LogLevel::DEBUG, "NodeCreatorWalker walker for_each");
        std::string nodeName = xmlNode.name();
        logger(LogLevel::DEBUG, "NodeCreatorWalker walker nodeName : " + nodeName);
        std::string id = xmlNode.attribute("id").as_string();
        std::string className = xmlNode.attribute("className").as_string();
        std::vector<std::string> classNames;
        classNames.push_back(className);

        Node* node;
        if (nodeName == "Node") {
            node = new Node(currentParent, id, classNames);
        } else if (nodeName == "Clickable") {
            node = new Clickable(currentParent, id, classNames);
        } else if (nodeName == "Button") {
            node = new Button(currentParent, id, classNames);
        } else {
            return false; // Stop traversal on unknown node type
        }
		
        const Style& style = styleManager.getStyle(id, className);
        node->setStyle(style);
        node->setTextContent(xmlNode.text().as_string());
		
		logger(LogLevel::DEBUG, "NodeCreatorWalker walker node id : " + id);
        logger(LogLevel::DEBUG, "NodeCreatorWalker walker className : " + className);
        logger(LogLevel::DEBUG, std::string("NodeCreatorWalker walker Node textContent is ") + xmlNode.text().as_string());
        logger(LogLevel::DEBUG, "set node className : " + node->classNames.at(0));

//        if (currentParent) {
//            currentParent->addChild(node);
//            logger(LogLevel::DEBUG, "NodeCreatorWalker walker child added");
//            logger(LogLevel::DEBUG, "currentParent child count: " + std::to_string(currentParent->children.size()));
//        }
//        else  {
//			rootNode->addChild(node);
//			logger(LogLevel::DEBUG, "NodeCreatorWalker walker added child to RootNode");
//			logger(LogLevel::DEBUG, "rootNode child count: " + std::to_string(rootNode->children.size()));
//		}
		
        currentParent = node;
        
        return true; // Continue traversal
    }
};
