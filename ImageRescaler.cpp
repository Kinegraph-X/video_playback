#include "ImageRescaler.h"

ImageRescaler::ImageRescaler() : swsContext(nullptr) {
	
}
ImageRescaler::~ImageRescaler() {
	cleanup();
}

void ImageRescaler::initializeSwsContext(AVCodecContext* codecContext, const WindowSize* initial_params) {
    if (!codecContext) {
        logger(LogLevel::ERR, "Cannot initialize SwsContext: codecContext is null.");
        return;
    }
	
    // Free the existing SwsContext if any.
    if (swsContext) {
		logger(LogLevel::DEBUG, "sws context already allocated, freeing it...");
        sws_freeContext(swsContext);
    }

    // Initialize the SwsContext for scaling/conversion.
    swsContext = sws_getContext(
        codecContext->width, codecContext->height, codecContext->pix_fmt, // Source parameters.
        initial_params->width, initial_params->height, AV_PIX_FMT_YUV420P,   // Target parameters.
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );

    if (!swsContext) {
        logger(LogLevel::ERR, "Failed to initialize SwsContext.");
    } else {
        logger(LogLevel::INFO, "SwsContext initialized successfully.");
    }
}

AVFrame* ImageRescaler::rescaleFrame(AVFrame* frame, const WindowSize& windowSize) {
    if (!frame) {
        logger(LogLevel::ERR, "Cannot rescale frame: frame is null.");
        return nullptr;
    }

    if (!swsContext) {
        logger(LogLevel::ERR, "SwsContext is not initialized. Ensure it is set up with the correct dimensions.");
        return nullptr;
    }

    // Verify dimensions match those used to initialize swsContext.
    if (windowSize.width <= 0 || windowSize.height <= 0) {
        logger(LogLevel::ERR, "Invalid target dimensions provided in WindowSize struct.");
        return nullptr;
    }

    // Allocate a new frame to hold the resized data.
    AVFrame* scaledFrame = av_frame_alloc();
    if (!scaledFrame) {
        logger(LogLevel::ERR, "Failed to allocate memory for scaled frame.");
        return nullptr;
    }

    // Set frame properties (format, width, height) using WindowSize dimensions.
    scaledFrame->format = AV_PIX_FMT_YUV420P;
    scaledFrame->width = windowSize.width;
    scaledFrame->height = windowSize.height;

    // Allocate buffer for the scaled frame.
    if (av_frame_get_buffer(scaledFrame, 32) < 0) {
        logger(LogLevel::ERR, "Failed to allocate buffer for scaled frame.");
        av_frame_free(&scaledFrame);
        return nullptr;
    }

    // Perform the scaling.
    int scaledHeight = sws_scale(
        swsContext,
        frame->data, frame->linesize,            // Source.
        0, frame->height,                       // Source slice.
        scaledFrame->data, scaledFrame->linesize // Destination.
    );

    if (scaledHeight <= 0) {
        logger(LogLevel::ERR, "sws_scale failed to rescale the frame.");
        av_frame_free(&scaledFrame);
        return nullptr;
    }

    return scaledFrame;
}

void ImageRescaler::cleanup() {
	sws_freeContext(swsContext);
}
    
void ImageRescaler::reset() {
    cleanup();  // Call cleanup to reset resources
}