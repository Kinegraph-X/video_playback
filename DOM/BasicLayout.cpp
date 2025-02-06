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
	currentTraversalDepth = 0;
	this->rootNode = root;
	this->windowSize.width = windowSize.width;
	this->windowSize.height = windowSize.height;
	renderableNodes->clear();
	parentDimensions.width = windowSize.width;
    parentDimensions.height = windowSize.height;
    root->computedStyle.bounds.value.width = windowSize.width;
    root->computedStyle.bounds.value.height = windowSize.height;
    logger(LogLevel::DEBUG, "Will make layout");
	traverseNodeTree(root, 0);
	sortNodeTree();
}

void BasicLayout::traverseNodeTree(Node* node, int depth) {
    if (!node) return;
	
	if (depth != 0) {
//		logger(LogLevel::DEBUG, "Updating parent dimensions on depth change");
//		logger(LogLevel::DEBUG, "Node has children : " + std::to_string(node->getChildren().size()));
    	parentDimensions = node->getParent()->computedStyle.bounds.value;
    	currentTraversalDepth = depth;
    	if (node->classNames.size() > 0) {
			logger(LogLevel::DEBUG, "layouting node of class " + node->classNames.at(0));
		}
    }
	    
//    logger(LogLevel::DEBUG, "Depth is " + std::to_string(depth));
    node->updateComputedStyle();
    RenderableNode* renderable = getRenderableNode(node, depth);
    if (depth == 0) {
		renderable->bounds.height = windowSize.height;
	}
    renderableNodes->push_back(renderable);

    for (const auto& child : node->getChildren()) {
//		if (depth == 0) resetParentDimensions();
        traverseNodeTree(child, depth + 1);
    }
}

RenderableNode* BasicLayout::getRenderableNode(Node* node, int depth) {
	RenderableNode* renderable = new RenderableNode;
    renderable->node = node;
    renderable->zIndex = node->getStyle().zIndex.value;
    renderable->depth = depth;
//    logger(LogLevel::DEBUG, "Will position node");
    positionNode(*renderable);
    renderable->bounds = node->computedStyle.bounds.value;
    logger(LogLevel::DEBUG, "renderable dimensions x : " + std::to_string(renderable->bounds.x));
	logger(LogLevel::DEBUG, "renderable dimensions y : " + std::to_string(renderable->bounds.y));
	logger(LogLevel::DEBUG, "renderable dimensions width : " + std::to_string(renderable->bounds.width));
	logger(LogLevel::DEBUG, "renderable dimensions height : " + std::to_string(renderable->bounds.height));
    return renderable;
}

void BasicLayout::positionNode(RenderableNode& renderable) {
	logger(LogLevel::DEBUG, "Parent dimensions x : " + std::to_string(parentDimensions.x));
	logger(LogLevel::DEBUG, "Parent dimensions y : " + std::to_string(parentDimensions.y));
	logger(LogLevel::DEBUG, "Parent dimensions width : " + std::to_string(parentDimensions.width));
	logger(LogLevel::DEBUG, "Parent dimensions height : " + std::to_string(parentDimensions.height));
	if (renderable.node->getStyle().position.value == Position::Relative) {
		logger(LogLevel::DEBUG, "Updating from parent dimensions");
		renderable.node->computedStyle.bounds.value.x += parentDimensions.x;
		renderable.node->computedStyle.bounds.value.y += parentDimensions.y;
	}
	
}

void BasicLayout::resetParentDimensions() {
	this->parentDimensions = {0, 0, static_cast<float>(windowSize.width), static_cast<float>(windowSize.height)};
}

std::vector<RenderableNode*>* BasicLayout::getFlattenedNodeTree(Node* root) {
	return renderableNodes;
}

void BasicLayout::cleanup () {
	for (RenderableNode* renderable : *renderableNodes) {
		if (renderable) {
			logger(LogLevel::DEBUG, "cleaning renderable");
			delete renderable;
		}
	}
	if (renderableNodes) {
		logger(LogLevel::DEBUG, "cleaning renderableNodes");
		delete renderableNodes;
	}
}

void BasicLayout::reset () {
	cleanup();
}
