#include "BasicLayout.h"




// Function to find the topmost node at a given position
Node* BasicLayout::findTargetNode(const RaylibVector2& position) {
    // Traverse the renderableNodes list in reverse order (top to bottom)
    for (auto it = renderableNodes.rbegin(); it != renderableNodes.rend(); ++it) {
        if (RaylibCheckCollisionPointRec(position, it->node->style->bounds)) {
            return it->node;
        }
    }
    return nullptr;  // No node found at the given position
}

void BasicLayout::flattenNodeTree(Node* root) {
    renderableNodes.clear();
    traverseNodeTree(root);

    // Sort the renderableNodes based on zIndex and depth
    std::sort(renderableNodes.begin(), renderableNodes.end(),
        [](const RenderableNode& a, const RenderableNode& b) {
            if (a.zIndex != b.zIndex) return a.zIndex < b.zIndex;
            return a.depth < b.depth;
        });
}

void BasicLayout::traverseNodeTree(Node* node, int depth = 0) {
    if (!node) return;
    renderableNodes.clear();

    RenderableNode renderable;
    renderable.node = node;
    renderable.zIndex = node->getStyle().zIndex;
    renderable.bounds = node->getStyle().bounds;
    renderable.depth = depth;

    renderableNodes.push_back(renderable);

    for (const auto& child : node->getChildren()) {
        traverseNodeTree(child.get(), depth + 1);
    }
}

std::vector<RenderableNode>* BasicLayout::getFlattenedNodeTree(Node* root) {
	return &renderableNodes;
}
