#include "../include/Logger.h"

#include <filesystem>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <mutex>

namespace securevault {

namespace fs = std::filesystem;

// Static mutex to guarantee thread-safe file appending
static std::mutex s_logMutex;

void Logger::ensureLogDirectoryExists() {
    if (!fs::exists(LOG_DIR)) {
        fs::create_directory(LOG_DIR);
    }
}

std::string Logger::getCurrentTimestamp() {
    // Fetch current system time
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    
    // Format to YYYY-MM-DD HH:MM:SS
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void Logger::log(std::string_view level, std::string_view message) {
    // Lock the mutex for the duration of this scope
    std::lock_guard<std::mutex> lock(s_logMutex);
    
    ensureLogDirectoryExists();

    // Open file in append mode (std::ios::app)
    std::ofstream logFile(LOG_FILE, std::ios::out | std::ios::app);
    
    if (logFile.is_open()) {
        logFile << "[" << getCurrentTimestamp() << "] " 
                << "[" << level << "] " 
                << message << "\n";
        logFile.close();
    } else {
        // Safe fallback if disk is full or permissions are denied
        std::cerr << "CRITICAL: Logger failed to open " << LOG_FILE << " for writing.\n";
    }
}

void Logger::logInfo(std::string_view message) {
    log("INFO", message);
}

void Logger::logWarning(std::string_view message) {
    log("WARN", message);
}

void Logger::logError(std::string_view message) {
    log("ERROR", message);
}

} // namespace securevault