#include "player_headers.h"

#include <string>
#include "PacketQueue.h"
#include "FrameQueue.h"
#include "AVFormatHandler.h"
#include "MediaState.h"
#include "DemuxThreadHandler.h"

struct MainThreadOptions {
    size_t minVideoPacketQueueSize = 10; 
    size_t maxVideoPacketQueueSize = 100;
    size_t minAudioPacketQueueSize = 10;
    size_t maxAudioPacketQueueSize = 100;
    size_t minVideoFrameQueueSize = 10;
    size_t maxVideoFrameQueueSize = 100;
    size_t minAudioFrameQueueSize = 10;
    size_t maxAudioFrameQueueSize = 100;
//    bool enableVideo = true;
//    bool enableAudio = true;
};




struct DemuxThreadData {
    DemuxThreadHandler* demuxThreadHandler;
    const MainThreadOptions* options;
};

class MainThreadHandlerHelpers {
public:
	static int demuxThreadFunction(void *data);
};





class MainThreadHandler {
private:
    PacketQueue& videoPacketQueue;
    PacketQueue& audioPacketQueue;
    FrameQueue& videoFrameQueue;
    FrameQueue& audioFrameQueue;
    AVFormatHandler& formatHandler;
    
    MediaState& mediaState;
//    SDL_ThreadID demuxThreadID;
//    SDL_ThreadID decodeThreadID;
    double currentTimestamp;
	double lastFrameTimestamp;
    double elapsedPlayingTime;
    double frameDuration;

    DemuxThreadHandler* demuxThreadHandler = nullptr;
    DemuxThreadData threadData;

    const MainThreadOptions& options;

public:
    MainThreadHandler(
		PacketQueue& videoPacketQueue,
	    PacketQueue& audioPacketQueue,
	    FrameQueue& videoFrameQueue,
	    FrameQueue& audioFrameQueue,
	    AVFormatHandler& formatHandler,
	    
	    MediaState& mediaState,
	    const MainThreadOptions& options
	);
    ~MainThreadHandler();

    void initialize();
    void mainLoop();

    void cleanUp();

private:
	double updateTiming();
    void createThreads();
};
