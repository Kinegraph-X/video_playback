#include "ImageRescaler.h"

ImageRescaler::ImageRescaler() {
	
}
ImageRescaler::~ImageRescaler() {
	cleanUp();
}

void ImageRescaler::initializeSwsContext(AVCodecContext* codecContext) {
	
    if (!codecContext) {
        logger(LogLevel::ERR, "Cannot initialize SwsContext: codecContext is null.");
        return;
    }

    // Free the existing SwsContext if any.
    if (swsContext) {
        sws_freeContext(swsContext);
    }

    // Initialize the SwsContext for scaling/conversion.
    swsContext = sws_getContext(
        codecContext->width, codecContext->height, codecContext->pix_fmt, // Source parameters.
        codecContext->width, codecContext->height, AV_PIX_FMT_YUV420P,   // Target parameters.
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );

    if (!swsContext) {
        logger(LogLevel::ERR, "Failed to initialize SwsContext.");
    } else {
        logger(LogLevel::INFO, "SwsContext initialized successfully.");
    }
}

AVFrame* ImageRescaler::rescaleFrame(AVFrame* frame, AVCodecContext* codecContext) {
	if (!frame || !codecContext) {
        logger(LogLevel::ERR, "Cannot rescale frame: frame or codecContext is null.");
        return nullptr;
    }

    if (!swsContext) {
        logger(LogLevel::ERR, "SwsContext is not initialized. Call initializeSwsContext first.");
        return nullptr;
    }

    // Allocate a new frame to hold the converted data.
    AVFrame* scaledFrame = av_frame_alloc();
    if (!scaledFrame) {
        logger(LogLevel::ERR, "Failed to allocate memory for scaled frame.");
        return nullptr;
    }

    // Set up the buffer for the scaled frame.
    int bufferSize = av_image_alloc(
        scaledFrame->data, scaledFrame->linesize,
        codecContext->width, codecContext->height, AV_PIX_FMT_YUV420P, 32
    );

    if (bufferSize < 0) {
        logger(LogLevel::ERR, "Failed to allocate buffer for scaled frame.");
        av_frame_free(&scaledFrame);
        return nullptr;
    }

    // Perform the scaling.
    sws_scale(
        swsContext,
        frame->data, frame->linesize,            // Source.
        0, codecContext->height,                // Source slice.
        scaledFrame->data, scaledFrame->linesize // Destination.
    );

    // Set frame properties.
    scaledFrame->format = AV_PIX_FMT_YUV420P;
    scaledFrame->width = codecContext->width;
    scaledFrame->height = codecContext->height;

    return scaledFrame;
}

void ImageRescaler::cleanUp() {
	sws_freeContext(swsContext);
}
    
