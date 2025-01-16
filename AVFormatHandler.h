#pragma once

#include "player_headers.h"
#include "utils.h"

class AVFormatHandler {
public:
    // Constructor and Destructor
    AVFormatHandler();
    ~AVFormatHandler();

    // Open a video file and initialize contexts
    bool openFile(const char *filePath);

    // Retrieve video and audio stream indices
    bool setVideoStream();
    bool setAudioStream();

    // Get pointers to codec contexts (video/audio)
    AVFormatContext* getFormatContext();
    AVCodecContext* getVideoCodecContext();
    AVCodecContext* getAudioCodecContext();
    
    int getVideoStreamIndex();
	int getAudioStreamIndex();

    // Retrieve stream time bases for synchronization
    AVRational getVideoTimeBase();
    AVRational getAudioTimeBase();
    
    void setAudioSampleRate();
    void setFrameDuration();
    void setAudioChannelLayout();
	void setAudioSampleFormat();
	void setDuration();
    
    bool openStreams();
    
    int initializeCodecContext(int *stream_idx, AVFormatContext *formatContext, AVCodecContext **codecContext, enum AVMediaType type);

    // Check if the handler is ready
    bool isReady();

    // Reset and release all resources
    void reset();


    // Libav components
    AVFormatContext* formatContext = nullptr;      // The main format context
    AVCodecContext* videoCodecContext = nullptr;  // Video codec context
    AVCodecContext* audioCodecContext = nullptr;  // Audio codec context
    AVCodecParameters* videoCodecPar = nullptr;  // Video codec parameters
    AVCodecParameters* audioCodecPar = nullptr;  // Audio codec parameters
    int videoStreamIndex;               // Index of the video stream
    int audioStreamIndex;               // Index of the audio stream
    AVStream* videoStream = nullptr;  // Pointer to the video stream
	AVStream* audioStream = nullptr;  // Pointer to the audio stream
//	videoDecoder;
//	audioDecoder;
	
	int audioSampleRate;
	double videoFrameDuration;
	AVChannelLayout* audioChannelLayout;
	AVSampleFormat sampleFormat;
	double duration = 0.;

    // Initialization flag
    bool initialized;

private:
    // Helper methods
    void cleanup(); // Helper for cleanup tasks
};
