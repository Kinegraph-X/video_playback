#include "BasicLayout.h"

BasicLayout::BasicLayout() {}
BasicLayout::~BasicLayout() {
	cleanup();
}

// Function to find the topmost node at a given position
//Node* BasicLayout::findTargetNode(const RaylibVector2& position) {
//    // Traverse the renderableNodes list in reverse order (top to bottom)
//    for (RenderableNode* it : *renderableNodes) {
//        if (RaylibCheckCollisionPointRec(position, it->node->style->bounds.value)) {
//            return it->node;
//        }
//    }
//    return nullptr;  // No node found at the given position
//}

void BasicLayout::sortNodeTree() {
    // Sort the renderableNodes based on zIndex and depth
    std::sort(renderableNodes->begin(), renderableNodes->end(),
        [](const RenderableNode* a, const RenderableNode* b) {
            if (a->zIndex != b->zIndex) return a->zIndex < b->zIndex;
            return a->depth < b->depth;
        });
}

void BasicLayout::makeLayout(RootNode* root, WindowSize windowSize) {
	this->rootNode = root;
	this->windowSize.width = windowSize.width;
	this->windowSize.height = windowSize.height;
	renderableNodes->clear();
	parentDimensions.width = windowSize.width;
    parentDimensions.height = windowSize.height;
    logger(LogLevel::DEBUG, "Will make layout");
	traverseNodeTree(root, 0);
	sortNodeTree();
}

void BasicLayout::traverseNodeTree(Node* node, int depth) {
    if (!node) return;
    
    RenderableNode* renderable = getRenderableNode(node, depth);
    if (depth == 0) {
		renderable->bounds.height = windowSize.height;
	}
    renderableNodes->push_back(renderable);
    parentDimensions = renderable->bounds;

    for (const auto& child : node->getChildren()) {
		if (depth == 0) resetParentDimensions();
        traverseNodeTree(child, depth + 1);
    }
}

RenderableNode* BasicLayout::getRenderableNode(Node* node, int depth) {
	RenderableNode* renderable = new RenderableNode;
    renderable->node = node;
    renderable->zIndex = node->getStyle().zIndex.value;
    renderable->bounds = node->getStyle().bounds.value;
    renderable->depth = depth;
//    logger(LogLevel::DEBUG, "Will position node");
    positionNode(*renderable);
    return renderable;
}

void BasicLayout::positionNode(RenderableNode& renderable) {
//	logger(LogLevel::DEBUG, "Parent dimensions x : " + std::to_string(parentDimensions.x));
	logger(LogLevel::DEBUG, "Parent dimensions y : " + std::to_string(parentDimensions.y));
//	logger(LogLevel::DEBUG, "Parent dimensions width : " + std::to_string(parentDimensions.width));
//	logger(LogLevel::DEBUG, "Parent dimensions height : " + std::to_string(parentDimensions.height));
	if (renderable.node->getStyle().position.value == Position::Relative) {
		logger(LogLevel::DEBUG, "HERE : ");
		renderable.bounds.x += parentDimensions.x;
		renderable.bounds.y += parentDimensions.y;
	}
	if (renderable.bounds.width == 0) {
		renderable.bounds.width = parentDimensions.width;
	}
	logger(LogLevel::DEBUG, "renderable dimensions x : " + std::to_string(renderable.bounds.x));
	logger(LogLevel::DEBUG, "renderable dimensions y : " + std::to_string(renderable.bounds.y));
	logger(LogLevel::DEBUG, "renderable dimensions width : " + std::to_string(renderable.bounds.width));
	logger(LogLevel::DEBUG, "renderable dimensions height : " + std::to_string(renderable.bounds.height));
}

void BasicLayout::resetParentDimensions() {
	this->parentDimensions = {0, 0, static_cast<float>(windowSize.width), static_cast<float>(windowSize.height)};
}

std::vector<RenderableNode*>* BasicLayout::getFlattenedNodeTree(Node* root) {
	return renderableNodes;
}

void BasicLayout::cleanup () {
	delete rootNode;	
	for (RenderableNode* renderable : *renderableNodes) {
		delete renderable;
	}
	if (renderableNodes) {
		delete renderableNodes;
	}
}

void BasicLayout::reset () {
	cleanup();
}
