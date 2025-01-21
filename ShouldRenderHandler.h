#pragma once

#include "player_headers.h"
#include "FrameQueue.h"

class ShouldRenderHandler {
public:
    explicit ShouldRenderHandler(FrameQueue& frameQueue);
    bool handleRenderEvent(void* eventData, AVFrame* frame);

private:
    FrameQueue& frameQueue;
};

