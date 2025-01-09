#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>

// Define logging levels
#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO 1
#define LOG_LEVEL_WARN 2
#define LOG_LEVEL_ERROR 3

// Set the current logging level
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_DEBUG
#endif

#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define LOG_ERROR(message) Logger::log("ERROR", message, __FUNCTION__, __FILE__, __LINE__)
#else
#define LOG_ERROR(message)
#endif

#if LOG_LEVEL <= LOG_LEVEL_WARN
#define LOG_WARN(message) Logger::log("WARN", message, __FUNCTION__, __FILE__, __LINE__)
#else
#define LOG_WARN(message)
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

/**
 * @brief Class for logging messages.
 */
class Logger {
public:
    /**
     * @brief Log a message.
     *
     * @param level The log level (e.g., "DEBUG", "INFO", "WARN", "ERROR").
     * @param message The message to log.
     * @param function The function name where the log is called.
     * @param file The file name where the log is called.
     * @param line The line number where the log is called.
     */
    static void log(const std::string& level, const std::string& message, const std::string& function, const std::string& file, int line) {
        std::cerr << "[" << level << "] " << file << ":" << line << " (" << function << ") - " << message << std::endl;
    }
};

#endif
