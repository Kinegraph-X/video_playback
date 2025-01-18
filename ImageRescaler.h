#pragma once

#include "player_headers.h"
#include "constants.h"

class ImageRescaler {
public:
    ImageRescaler();
    ~ImageRescaler();
    
    void initializeSwsContext(AVCodecContext* codecContext, const WindowSize* initial_params);
    AVFrame* rescaleFrame(AVFrame* frame, const WindowSize& windowSize);
    void reset();
private:
	struct SwsContext* swsContext = nullptr;
	void cleanup();
};