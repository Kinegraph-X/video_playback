#include "RaylibManager.h"


RaylibManager::RaylibManager(int width, int height, int titleHeight, int uiHeight, BasicLayout layoutInstance)
    : windowWidth(width), windowHeight(height), titleBarHeight(titleHeight), uiBarHeight(uiHeight), basicLayout(layoutInstance) {
		
    RaylibInitWindow(windowWidth, windowHeight, "RaylibManager Example");
    // Initialize video texture
    videoTexture = RaylibLoadTextureFromImage(RaylibGenImageColor(windowWidth, windowHeight - titleBarHeight - uiBarHeight, RAYLIB_BLACK));
}

void RaylibManager::reflectWindowResizeOnVideo() {
    // Unload the previous video texture
    RaylibUnloadTexture(videoTexture);

    // Calculate new video texture dimensions
    int videoHeight = windowHeight - titleBarHeight - uiBarHeight;
    videoTexture = RaylibLoadTextureFromImage(RaylibGenImageColor(windowWidth, videoHeight, RAYLIB_BLACK));
}

void RaylibManager::resizeWindow(const WindowSizeOffset &offset) {
    // Update window dimensions
    windowWidth += offset.xOffset;
    windowHeight += offset.yOffset;

    // Ensure minimum dimensions
    if (windowWidth < 100) windowWidth = 100;
    if (windowHeight < titleBarHeight + uiBarHeight + 100) windowHeight = titleBarHeight + uiBarHeight + 100;

    // Resize the window
    RaylibSetWindowSize(windowWidth, windowHeight);

    // Reflect changes on video texture
    reflectWindowResizeOnVideo();
}

void RaylibManager::acquireRenderableNodes(std::vector<RenderableNode>* nodes) {
	renderableNodes = nodes;
}

void RaylibManager::render() {
    RaylibBeginDrawing();
    RaylibClearBackground(RAYLIB_RAYWHITE);

    // Render all nodes in the renderableNodes vector
    for (const auto& renderable : *renderableNodes) {
        renderNode(renderable.node);
    }

    // Force raylib to draw its internal batch before we potentially add ImGui
    rlDrawRenderBatchActive();

    // If using ImGui, render it here
    // ImGui::Render();
    // ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    RaylibEndDrawing();
}

void RaylibManager::renderNode(Node* node) {
    Style& style = node->getStyle();

    // Draw background
    RaylibDrawRectangleRec(style.bounds, style.backgroundColor);

    // Draw border
    if (style.borderWidth > 0) {
        RaylibDrawRectangleLinesEx(style.bounds, style.borderWidth, style.borderColor);
    }

    // Draw text content
    RaylibDrawText(node->getTextContent().c_str(),
             style.bounds.x, style.bounds.y,
             style.fontSize, style.textColor);

    // If node has a background image, draw it
    if (!style.backgroundImage.empty()) {
        // Assuming we have a method to load/cache textures
        Texture2D texture = textureCache.getTexture(style.backgroundImage);
        RaylibDrawTextureRec(texture, style.bounds, {style.bounds.x, style.bounds.y}, RAYLIB_WHITE);
    }

    // Render child nodes recursively
    for (auto* child : node->getChildren()) {
        renderNode(child);
    }
}

void RaylibManager::dispatchEvent(const RaylibVector2& position, const EventPayload& payload) {
    Node* target = basicLayout.findTargetNode(position);
    if (target) {
        target->dispatchEvent(payload);
    }
}


void RaylibManager::cleanup() {
    RaylibUnloadTexture(videoTexture);
    RaylibCloseWindow();
}

RaylibManager::~RaylibManager() {
    cleanup();
}
