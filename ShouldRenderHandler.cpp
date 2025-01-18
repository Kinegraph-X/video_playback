#include "ShouldRenderHandler.h"

ShouldRenderHandler::ShouldRenderHandler(FrameQueue& frameQueue)
    : frameQueue(frameQueue) {}

bool ShouldRenderHandler::handleRenderEvent(void* eventData, AVFrame* frame) {

    PlayerEvent* playerEvent = static_cast<PlayerEvent*>(eventData);
    if (!playerEvent) {
        logger(LogLevel::ERR, "PlayerEvent is null.");
        return false;
    }

    logger(LogLevel::INFO, "Processing SHOULD_RENDER event. PTS: " +
                           LogUtils::toString(playerEvent->elapsedPlaybackTime));

    if (!frame) {
        logger(LogLevel::ERR, "Rnrefed frame pass to handleRenderEvent has not been allocated.");
        delete playerEvent; // Clean up memory.
        return false;
    }

    if (!frameQueue.get(frame)) {
        logger(LogLevel::ERR, "Failed to retrieve a frame from the queue.");
        av_frame_free(&frame);
        delete playerEvent; // Clean up memory.
        return false;
    }

    logger(LogLevel::DEBUG, "Frame retrieved: Width=" + LogUtils::toString(frame->width) +
                            ", Height=" + LogUtils::toString(frame->height));

    
    delete playerEvent; // Clean up memory.
    return true;
}
