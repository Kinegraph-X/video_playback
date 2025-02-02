#pragma once
#include "player_headers.h"
#include "ImageRescaler.h"
#include "DOM/Node.h"
#include "DOM/BasicLayout.h"
#include "DOM/TextureCache.h"

struct WindowSizeOffset {
    int xOffset;
    int yOffset;
};

struct RoundedRectangleParams {
    float roundness = 0;
    int segments = 0;
};

class RaylibManager {
private:
	bool aborted = false;
    int windowWidth;
    int windowHeight;
    int videoTextureWidth;
    int videoTextureHeight;
    int titleBarHeight;
    int uiBarHeight;
    uint8_t* frameBuffer = nullptr;
	
	BasicLayout& layout;
	ImageRescaler& rescaler;
	TextureCache textureCache;
	std::vector<RenderableNode*>* renderableNodes = nullptr;
    Texture2D videoTexture;
    RenderTexture2D titleBarContainer;
    RenderTexture2D uiBarContainer;

    void reflectWindowResizeOnVideo();
    RoundedRectangleParams calculateRoundedRectangleParams(RaylibRectangle bounds, int borderRadius);
//    void dispatchEvent(const RaylibVector2& position, const EventPayload& payload);

public:
    RaylibManager(int width, int height, int titleHeight, int uiHeight, BasicLayout& layoutInstance, ImageRescaler& rescaler, char* title);

    void resizeWindow(const WindowSizeOffset &offset);
    void resizeWindowFileLoaded(AVCodecContext* codecContext, const WindowSize* initialParams);
    void resetRescaler(AVCodecContext* codecContext, const WindowSize initialParams);
    void copyFrameDataWithoutPadding(AVFrame* scaledFrame);

    void render(bool shouldEndDrawing = true);
	
	void renderFrame(AVFrame *frame);
    void renderNode(Node* node);

	void acquireRenderableNodes(std::vector<RenderableNode>* renderableNodes);
	
    void cleanup();

    ~RaylibManager();
};
