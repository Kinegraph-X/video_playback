#include "logger.h"
#include "utils.h"
#include "MediaState.h"


std::string logLevelToString(LogLevel level) {
    switch (level) {
        case TRACE: return "TRACE";
        case DEBUG: return "DEBUG";
        case INFO:  return "INFO";
        case WARN:  return "WARN";
        case ERR:   return "ERROR";
        default:    return "UNKNOWN";
    }
}


Logger& Logger::getInstance() {
	static Logger instance;
	return instance;
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> guard(logMutex);

	time_t now = time(0);
	std::string timeStr = ctime(&now);
	timeStr.erase(std::remove(timeStr.begin(), timeStr.end(), '\n'), timeStr.end());
	logFile << timeStr << " : [" << logLevelToString(level) << "] " << message << std::endl;
}


Logger::Logger() {
	std::string basePath = GetExecutablePath();
    logFile.open(basePath + "\\app_log.txt", std::ios::trunc);
} // Private constructor

Logger::~Logger() {}



namespace LogUtils {
	template<typename T>
    std::string LogUtils::toString(const T& value) {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    };
    
    template<>
    std::string toString<float>(const float& value) {
        std::ostringstream oss;
        oss.precision(4); // 4 decimal places
        oss << std::fixed << value;
        return oss.str();
    }

    template<>
    std::string toString<double>(const double& value) {
        std::ostringstream oss;
        oss.precision(4); // 4 decimal places
        oss << std::fixed << value;
        return oss.str();
    }
    
    template <typename MediaState::State>
    typename std::enable_if<std::is_enum<MediaState::State>::value, std::string>::type
    toString(const MediaState::State& value) {
        using UnderlyingType = typename std::underlying_type<MediaState::State>::type;
        return toString(static_cast<UnderlyingType>(value));
    }
    
    // Specialization for enums
//    template <typename T>
//    typename std::enable_if<std::is_enum<T>::value, std::string>::type
//    toString(const T& value) {
//        using UnderlyingType = typename std::underlying_type<T>::type;
//        return toString(static_cast<UnderlyingType>(value));
//    }
    template std::string toString<MediaState::State>(const MediaState::State& value);
//    template std::string toString<AVSampleFormat>(const AVSampleFormat& value);

    // Explicit instantiations for other types
	template std::string toString<char>(const char&);
    template std::string toString<size_t>(const size_t&);
    template std::string toString<int>(const int&);
    template std::string toString<unsigned short>(const unsigned short&);
    template std::string toString<bool>(const bool&);
    template std::string toString<std::atomic<bool>>(const std::atomic<bool>&);
    template std::string toString<uint8_t>(const uint8_t&);
    template std::string toString<int64_t>(const int64_t&);
    template std::string toString<std::string>(const std::string&);

    std::string LogUtils::avPacketToString(const AVPacket* pkt) {
        // Implement conversion logic
        return "AVPacket: size=" + toString(pkt->size);
    }

    std::string LogUtils::avFrameToString(const AVFrame* frame) {
        // Implement conversion logic
        return "AVFrame: width=" + toString(frame->width) + ", height=" + toString(frame->height);
    }
}
