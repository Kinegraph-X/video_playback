#pragma once
#include "player_headers.h"
#include "DOM/Node.h"
#include "DOM/BasicLayout.h"
#include "DOM/TextureCache.h"

struct WindowSizeOffset {
    int xOffset;
    int yOffset;
};

struct Zone {
    RaylibRectangle bounds;
};

class RaylibManager {
private:
    int windowWidth;
    int windowHeight;
    int titleBarHeight;
    int uiBarHeight;
	
	BasicLayout& basicLayout;
	TextureCache textureCache;
	std::vector<RenderableNode>* renderableNodes;
    Texture2D videoTexture;
    RenderTexture2D titleBarContainer;
    RenderTexture2D uiBarContainer;

    std::vector<Zone> zones;

    void reflectWindowResizeOnVideo();
    void dispatchEvent(const RaylibVector2& position, const EventPayload& payload);

public:
    RaylibManager(int width, int height, int titleHeight, int uiHeight, BasicLayout layoutInstance);

    void resizeWindow(const WindowSizeOffset &offset);

    void render();

    void renderNode(Node* node);

	void acquireRenderableNodes(std::vector<RenderableNode>* renderableNodes);
	
    void cleanup();

    ~RaylibManager();
};
