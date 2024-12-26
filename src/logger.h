#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>

// Define logging levels
#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO 1
#define LOG_LEVEL_ERROR 2

// Set the current logging level
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_DEBUG
#endif

#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define LOG_ERROR(message) Logger::log("ERROR", message, __FUNCTION__, __FILE__, __LINE__)
#else
#define LOG_ERROR(message)
#endif

#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOG_INFO(message) Logger::log("INFO", message, __FUNCTION__, __FILE__, __LINE__)
#else
#define LOG_INFO(message)
#endif

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LOG_DEBUG(message) Logger::log("DEBUG", message, __FUNCTION__, __FILE__, __LINE__)
#else
#define LOG_DEBUG(message)
#endif

class Logger {
public:
    static void log(const std::string& level, const std::string& message, const std::string& function, const std::string& file, int line) {
        std::cerr << "[" << level << "] " << file << ":" << line << " (" << function << ") - " << message << std::endl;
    }
};

#endif
