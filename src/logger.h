#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>

#define LOG_ERROR(message) Logger::log("ERROR", message, __FUNCTION__, __FILE__, __LINE__)
#define LOG_DEBUG(message) Logger::log("DEBUG", message, __FUNCTION__, __FILE__, __LINE__)
#define LOG_INFO(message) Logger::log("INFO", message, __FUNCTION__, __FILE__, __LINE__)

class Logger {
public:
    static void log(const std::string& level, const std::string& message, const std::string& function, const std::string& file, int line) {
        std::cerr << "[" << level << "] " << file << ":" << line << " (" << function << ") - " << message << std::endl;
    }
};

#endif
