#pragma once
#include "player_headers.h"
#include "DOM/Node.h"

struct RenderableNode {
    Node* node;
    int zIndex;
    float yPosition;
};


class BasicLayout {
public:
	std::vector<RenderableNode> renderableNodes;

	void createFlattenedNodeTree(Node* root);

	// Function to find the topmost node at a given position
	Node* findTargetNode(const RaylibVector2& position);
	
	std::vector<RenderableNode>* getFlattenedNodeTree(Node* root);
	
private:
	void flattenNodeTree(Node* root);
	void traverseNodeTree(Node* node, int depth = 0);
};
