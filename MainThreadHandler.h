#include "player_headers.h"

#include <string>
#include <thread>
#include "constants.h"
#include "PacketQueue.h"
#include "FrameQueue.h"
#include "AVFormatHandler.h"
#include "SDLAudioDevice.h"
#include "MediaState.h"
#include "DemuxThreadHandler.h"
#include "DecodeThreadHandler.h"

struct MainThreadOptions {
    size_t minVideoPacketQueueSize = 30; 
    size_t maxVideoPacketQueueSize = 60;
    size_t minAudioPacketQueueSize = 80;
    size_t maxAudioPacketQueueSize = 160;
    
    size_t seekMinVideoPacketQueueSize = 1; 
    size_t seekMaxVideoPacketQueueSize = 60;
    size_t seekMinAudioPacketQueueSize = 1;
    size_t seekMaxAudioPacketQueueSize = 160;
    
    size_t minVideoFrameQueueSize = 10;
    size_t maxVideoFrameQueueSize = 30;
    size_t minAudioFrameQueueSize = 25;
    size_t maxAudioFrameQueueSize = 80;
    
    size_t seekMinVideoFrameQueueSize = 1;
    size_t seekMaxVideoFrameQueueSize = 30;
    size_t seekMinAudioFrameQueueSize = 1;
    size_t seekMaxAudioFrameQueueSize = 80;
    
    size_t minAudioDeviceQueueSize = 25;	// defined in the initializer function, based on the actual frame duration
//    bool enableVideo = true;
//    bool enableAudio = true;
};




struct DemuxThreadData {
    DemuxThreadHandler* demuxThreadHandler;
    const MainThreadOptions* options;
};
struct DecodeThreadData {
    DecodeThreadHandler* decodeThreadHandler;
    const MainThreadOptions* options;
};

class MainThreadHandlerHelpers {
public:
	static int demuxThreadFunction(void *data);
	static int decodeThreadFunction(void *data);
};





class MainThreadHandler {
private:
    PacketQueue& videoPacketQueue;
    PacketQueue& audioPacketQueue;
    FrameQueue& videoFrameQueue;
    FrameQueue& audioFrameQueue;
    AVFormatHandler& formatHandler;
    AudioDevice* audioDevice;
	
//	SDL_Thread* demuxThread = nullptr;
//	SDL_Thread* decodeThread = nullptr;
	
	std::thread demuxThread;
	std::thread decodeThread;
	
    double currentTimestamp;
	double lastFrameTimestamp;
    double elapsedPlayingTime;
    double frameDuration;
    
    bool playRequested = false;
    bool pauseRequested = false;
    bool stopRequested = false;
    bool seekRequested = false;
    double enqueuedAudioDuration = 0.;

    DemuxThreadHandler* demuxThreadHandler = nullptr;
    DemuxThreadData demuxThreadData;
    DecodeThreadHandler* decodeThreadHandler;
    DecodeThreadData decodeThreadData;

    MainThreadOptions& options;
    
    bool abort = false;
    mutable std::mutex mutex;

public:
    MainThreadHandler(
		PacketQueue& videoPacketQueue,
	    PacketQueue& audioPacketQueue,
	    FrameQueue& videoFrameQueue,
	    FrameQueue& audioFrameQueue,
	    AVFormatHandler& formatHandler,
	    AudioDevice* audioDevice,
	    
	    MediaState& mediaState,
	    MainThreadOptions& options
	);
    ~MainThreadHandler();
    
    MediaState& mediaState;

    bool initialize();
    
    void setAbort(bool value);
    bool isAborted() const;
    void cleanUp();
    
    void play();
	void pause();
	void stop();
	void seek(double position);

private:
	void manageAudioQueue();
	double updateTiming();
	void manageStatus();
    void createThreads();
    void resetQueues();
    void mainLoop();
    void setMinAudioDeviceQueueSize();
    SDL_Event newEvent(PlayerEvent::Type);
    void logStatus(MediaState::State status, std::string origin);
};
