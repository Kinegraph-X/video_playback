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
    
    int videoTextureWidth;
    int videoTextureHeight;
    int titleBarHeight;
    int uiBarHeight;
    uint8_t* frameBuffer = nullptr;
	
	BasicLayout& layout;
	ImageRescaler& rescaler;
	TextureCache* textureCache = new TextureCache();
	std::vector<RenderableNode*>* renderableNodes = nullptr;
    Texture2D videoTexture;
    RenderTexture2D titleBarContainer;
    RenderTexture2D uiBarContainer;

    void reflectWindowResizeOnVideo();
    RoundedRectangleParams calculateRoundedRectangleParams(RaylibRectangle bounds, int borderRadius);
//    void dispatchEvent(const RaylibVector2& position, const EventPayload& payload);

public:
	WindowSize windowPosition;
	int windowWidth;
    int windowHeight;
    
    RaylibManager(int x, int y, int width, int height, int titleHeight, int uiHeight, BasicLayout& layoutInstance, ImageRescaler& rescaler, char* title);

	void moveWindow(RaylibVector2 &offset);
    void resizeWindow(WindowSizeOffset &offset);
    void resizeWindowFileLoaded(AVCodecContext* codecContext, WindowSize& initialParams);
    void resetRescaler(AVCodecContext* codecContext, WindowSize initialParams);
    void copyFrameDataWithoutPadding(AVFrame* scaledFrame);

    void render(bool shouldEndDrawing = true);
	
	void renderFrame(AVFrame *frame);
    void renderNode(const RenderableNode* renderable);

	void acquireRenderableNodes(std::vector<RenderableNode>* renderableNodes);
	
    void cleanup();

    ~RaylibManager();
};
