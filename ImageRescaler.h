#pragma once

#include "player_headers.h"

class ImageRescaler {
public:
    ImageRescaler();
    ~ImageRescaler();
    
    void initializeSwsContext(AVCodecContext* codecContext);
    AVFrame* rescaleFrame(AVFrame* frame, AVCodecContext* codecContext);
    void cleanUp();
    
private:
	SwsContext* swsContext = nullptr;
	
};