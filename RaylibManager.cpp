#include "RaylibManager.h"


RaylibManager::RaylibManager(int width, int height, int titleHeight, int uiHeight, BasicLayout& layoutInstance, ImageRescaler& resizer, char* title)
    : windowWidth(width), windowHeight(height), titleBarHeight(titleHeight), uiBarHeight(uiHeight), layout(layoutInstance), rescaler(resizer) {
	
	renderableNodes = layout.renderableNodes;
	RaylibSetConfigFlags(RAYLIB_FLAG_WINDOW_TRANSPARENT);	
    RaylibInitWindow(windowWidth, windowHeight, title);
    
    // Initialize video texture
    reflectWindowResizeOnVideo();
    RaylibBeginDrawing();
    RaylibClearBackground(RAYLIB_BLANK);
    RaylibEndDrawing();
}

void RaylibManager::reflectWindowResizeOnVideo() {
    // Unload the previous video texture
    RaylibUnloadTexture(videoTexture);

    // Calculate new video texture dimensions
    videoTextureWidth = windowWidth;
    videoTextureHeight = windowHeight - titleBarHeight - uiBarHeight;
    RaylibImage image = RaylibGenImageColor(videoTextureWidth, videoTextureHeight, RAYLIB_BLANK);
    RaylibImageFormat(&image, RAYLIB_PIXELFORMAT_UNCOMPRESSED_R8G8B8); 
    videoTexture = RaylibLoadTextureFromImage(image);
    RaylibUnloadImage(image);
}

void RaylibManager::resizeWindow(const WindowSizeOffset &offset) {
    // Update window dimensions
    windowWidth += offset.xOffset;
    windowHeight += offset.yOffset;

    // Ensure minimum dimensions
    if (windowWidth < 100) windowWidth = 100;
    if (windowHeight < titleBarHeight + uiBarHeight + 100) windowHeight = titleBarHeight + uiBarHeight + 100;

    RaylibSetWindowSize(windowWidth, windowHeight);
    reflectWindowResizeOnVideo();
}

void RaylibManager::resizeWindowFileLoaded(AVCodecContext* codecContext, const WindowSize* windowSize) {
	windowWidth = windowSize->width;
	windowHeight = windowSize->height;
	reflectWindowResizeOnVideo();
    const WindowSize textureSize{videoTextureWidth, videoTextureHeight};
	resetRescaler(codecContext, textureSize);
}

void RaylibManager::resetRescaler(AVCodecContext* codecContext, const WindowSize windowSize) {
	rescaler.initializeSwsContext(codecContext, &windowSize);
	frameBuffer = new uint8_t[windowSize.width * windowSize.height * 3]();
}

void RaylibManager::copyFrameDataWithoutPadding(AVFrame* scaledFrame) {
    if (!scaledFrame || !frameBuffer) {
        logger(LogLevel::ERR, "Invalid frame or buffer pointer.");
        return;
    }

    int srcStride = scaledFrame->linesize[0];  // Source stride (includes padding)
    int dstStride = videoTextureWidth * 3;  // Expected stride (packed RGB format)

    uint8_t* srcPtr = scaledFrame->data[0];  // Start of frame data
    uint8_t* dstPtr = frameBuffer;  // Destination buffer

    // Copy each row, ignoring the padding
    for (int y = 0; y < videoTextureHeight; y++) {
        memcpy(dstPtr, srcPtr, dstStride);  // Copy only the valid pixels
        srcPtr += srcStride;  // Move to the next row in the source (respecting stride)
        dstPtr += dstStride;  // Move to the next row in the destination
    }

//    logger(LogLevel::DEBUG, "Frame copied without padding. Width: " + std::to_string(videoTexture.width) + ", Height: " + std::to_string(videoTexture.height));
}


void RaylibManager::render(bool shouldEndDrawing) {
    RaylibBeginDrawing();
    RaylibClearBackground(RAYLIB_BLANK);

    // Render all nodes in the renderableNodes vector
    for (const auto* renderable : *renderableNodes) {
		renderable->node->updateComputedStyle();
        renderNode(renderable);
    }
    if (shouldEndDrawing) {
		RaylibEndDrawing();
	}
}

void RaylibManager::renderFrame(AVFrame* frame) {
    if (!frame) return;
    
    // Render the UI
    render(false);
    
    
    WindowSize windowSize {
		windowWidth,
		windowHeight
	};

    // Ensure frame matches texture dimensions
    if (frame->format != AV_PIX_FMT_RGB24 || frame->width != videoTexture.width || frame->height != videoTexture.height) {
		AVFrame* scaledFrame = rescaler.rescaleFrame(frame, windowSize);
//		logger(LogLevel::DEBUG, "Width of the rescaled frame : " + std::to_string(scaledFrame->width));
//		logger(LogLevel::DEBUG, "Height of the rescaled frame : " + std::to_string(scaledFrame->height));
//		logger(LogLevel::DEBUG, "Width of the texture : " + std::to_string(videoTexture.width));
//		logger(LogLevel::DEBUG, "Height of the texture : " + std::to_string(videoTexture.height));
//		logger(LogLevel::DEBUG, "Pixel format of the frame : " + std::to_string(scaledFrame->format));
//		logger(LogLevel::DEBUG, "plane 0 size of the line : " + std::to_string(scaledFrame->linesize[0]));
//		logger(LogLevel::DEBUG, "plane 1 size of the line : " + std::to_string(scaledFrame->linesize[1]));
//		logger(LogLevel::DEBUG, "plane 2 size of the line : " + std::to_string(scaledFrame->linesize[2]));
//		logger(LogLevel::DEBUG, "plane 0 size of the buffer : " + std::to_string(sizeof(*scaledFrame->data[0])));
//		logger(LogLevel::DEBUG, "plane 1 size of the buffer : " + std::to_string(sizeof(*scaledFrame->data[1])));
//		logger(LogLevel::DEBUG, "plane 2 size of the buffer : " + std::to_string(sizeof(*scaledFrame->data[2])));
//		logger(LogLevel::DEBUG, "Pixel format of videoTexture: " + std::to_string(videoTexture.format));

//		if (!scaledFrame->data[0]) {
//		    logger(LogLevel::ERR, "scaledFrame->data[0] is NULL! Cannot update texture.");
//		}
		
//		logger(LogLevel::DEBUG, "Attempting to update texture...");
//		logger(LogLevel::DEBUG, "Memory address of frame data: " + std::to_string(reinterpret_cast<uintptr_t>(scaledFrame->data[0])));
//		try {
//		    volatile uint8_t testRead = *(scaledFrame->data[0]); // Attempt to read the first byte
//		    logger(LogLevel::DEBUG, "Memory read test passed: " + std::to_string(testRead));
//		} catch (...) {
//		    logger(LogLevel::ERR, "Memory read test failed! Possible invalid pointer access.");
//		    return;
//		}
		copyFrameDataWithoutPadding(scaledFrame);
		RaylibUpdateTexture(videoTexture, frameBuffer);
    }
    else {
		copyFrameDataWithoutPadding(frame);
		RaylibUpdateTexture(videoTexture, frameBuffer);		
	}

    // Begin drawing is called in the "render()" method

    // Draw video
    RaylibDrawTextureRec(videoTexture, {0, 0, (float)videoTexture.width, (float)videoTexture.height}, {0, 0}, RAYLIB_WHITE);

    RaylibEndDrawing();
}

void RaylibManager::renderNode(const RenderableNode* renderable) {
	Node* node = renderable->node;
    ComputedStyle& style = node->getComputedStyle();
	
	RoundedRectangleParams rectangleParams;
    // Draw background
    if (style.borderRadius.value > 0) {
		rectangleParams = calculateRoundedRectangleParams(renderable->bounds, style.borderRadius.value);
    	RaylibDrawRectangleRounded(
			renderable->bounds,
			rectangleParams.roundness,
			rectangleParams.segments, 
			style.backgroundColor.value
		);
    }
    else {
		RaylibDrawRectangleRec(renderable->bounds, style.backgroundColor.value);
	}

    // If node has a background image, draw it
    if (!style.backgroundImage.value.empty()) {
//		logger(LogLevel::DEBUG, "Retrieving texture from cache");
        Texture2D texture = textureCache->getTexture(style.backgroundImage.value);
        RaylibDrawTextureRec(texture, { 0, 0, (float)texture.width, (float)texture.height }, {renderable->bounds.x, renderable->bounds.y}, RAYLIB_WHITE);
    }
    
    // Draw border
    if (style.borderWidth.value > 0) {
		if (style.borderRadius.value > 0) {
			if (rectangleParams.roundness == 0) {
				rectangleParams = calculateRoundedRectangleParams(renderable->bounds, style.borderRadius.value);
			}
			RaylibDrawRectangleRoundedLinesEx(
				renderable->bounds,
				rectangleParams.roundness,
				rectangleParams.segments,
				style.borderWidth.value,
				style.borderColor.value
			);
		}
		else {
	        RaylibDrawRectangleLinesEx(
				renderable->bounds,
				style.borderWidth.value,
				style.borderColor.value
			);
		}
    }
    
    // Draw text content
    RaylibDrawText(
		node->getTextContent().c_str(),
		renderable->bounds.x,
		renderable->bounds.y,
		style.fontSize.value,
		style.textColor.value
	);

}

RoundedRectangleParams RaylibManager::calculateRoundedRectangleParams(RaylibRectangle bounds, int borderRadius) {
    RoundedRectangleParams params;

	#if defined(_WIN32)
    params.roundness = max(0.0f, min(1.0f, static_cast<float>(borderRadius) / min(bounds.width, bounds.height)));
    params.segments = max(4, min(32, borderRadius / 2));
    #else
    params.roundness = std::max(0.0f, std::min(1.0f, static_cast<float>(borderRadius) / std::min(bounds.width, bounds.height)));
    params.segments = std::max(4, std::min(32, borderRadius / 2))
    #endif

    return params;
}


//void RaylibManager::dispatchEvent(const RaylibVector2& position, const EventPayload& payload) {
//    Node* target = layout.findTargetNode(position);
//    if (target) {
//        target->dispatchEvent(payload);
//    }
//}


void RaylibManager::cleanup() {
	aborted = true;
	if (frameBuffer) {
		delete[] frameBuffer;
	}
	delete textureCache;
    RaylibUnloadTexture(videoTexture);
}

RaylibManager::~RaylibManager() {
	if (!aborted) cleanup();
}
