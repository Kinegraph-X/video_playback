#pragma once
#include  "../player_headers.h"
#include "Button.h"
#include "Checkbox.h"

struct NodeCreatorWalker : pugi::xml_tree_walker {
    Node* currentParent;
    const StyleManager& styleManager;

    NodeCreatorWalker(const StyleManager& sm) : currentParent(nullptr), styleManager(sm) {}

    virtual bool for_each(pugi::xml_node& xmlNode) {
        std::string nodeName = xmlNode.name();
        std::string id = xmlNode.attribute("id").as_string();
        std::string className = xmlNode.attribute("className").as_string();

        Node* node;
        if (nodeName == "Node") {
            node = new Node(currentParent, id, className);
        } else if (nodeName == "Button") {
            node = new Button(currentParent, id, className);
        } else {
            return false; // Stop traversal on unknown node type
        }

        const Style& style = styleManager.getStyle(id, className);
        node->setStyle(style);
        node->setTextContent(xmlNode.text().as_string());

        if (currentParent) {
            currentParent->addChild(std::unique_ptr<Node>(node));
        }

        currentParent = node;
        return true; // Continue traversal
    }
};
