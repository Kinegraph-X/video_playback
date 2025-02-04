#pragma once
#include "player_headers.h"  // IWYU pragma: export
#include "DOM/Node.h"
#include "DOM/RootNode.h"

struct RenderableNode {
    Node* node = nullptr;
    int zIndex = 0;
    RaylibRectangle bounds;
    int depth = 0;
};


class BasicLayout {
public:
	RootNode* rootNode = nullptr;
	BasicLayout();
	~BasicLayout();
	std::vector<RenderableNode*>* renderableNodes = new std::vector<RenderableNode*>();

	void makeLayout(RootNode* root, WindowSize windowSize);

	// Function to find the topmost node at a given position
//	Node* findTargetNode(const RaylibVector2& position);
	
	std::vector<RenderableNode*>* getFlattenedNodeTree(Node* root);
	void reset();
	
private:
	WindowSize windowSize;
	RaylibRectangle parentDimensions{0, 0, 0, 0};
	void sortNodeTree();
	void traverseNodeTree(Node* node, int depth = 0);
	RenderableNode* getRenderableNode(Node* node, int depth);
	void positionNode(RenderableNode& renderable);
	void resetParentDimensions();
	void cleanup();
};
