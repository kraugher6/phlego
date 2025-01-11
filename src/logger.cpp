#include "logger.h"

// Initialize the static mutex
std::mutex Logger::log_mutex;

void Logger::log(const std::string& level, const std::string& message, const std::string& function, const std::string& file, int line) {
    std::lock_guard<std::mutex> guard(log_mutex);
    std::cerr << "[" << level << "] " << file << ":" << line << " (" << function << ") - " << message << std::endl;
}
