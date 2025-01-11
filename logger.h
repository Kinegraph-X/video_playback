#pragma once

#include "player_headers.h"
#include <fstream>
#include <string>
#include <mutex>
#include <ctime>
#include <algorithm>
#include <sstream>

enum LogLevel { 
    TRACE,
    DEBUG, 
    INFO, 
    WARN, 
    ERR 
};

std::string logLevelToString(LogLevel level);


class Logger {
public:
    static Logger& getInstance();
    void log(LogLevel level, const std::string& message);

private:
	std::ofstream logFile;
	std::mutex logMutex;

    Logger(); // Private constructor
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

namespace LogUtils {
    template<typename T>
    std::string toString(const T& value);
    
	
    std::string avPacketToString(const AVPacket* pkt);

    std::string avFrameToString(const AVFrame* frame);
}

#define logger(level, message) Logger::getInstance().log(level, message)