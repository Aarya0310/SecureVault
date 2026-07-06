#pragma once

#include <string>
#include <string_view>

namespace securevault {

class Logger {
public:
    // This is a static utility class; instantiation is forbidden
    Logger() = delete;

    // Core Logging Interface
    static void logInfo(std::string_view message);
    static void logWarning(std::string_view message);
    static void logError(std::string_view message);

private:
    static constexpr const char* LOG_DIR = "logs/";
    static constexpr const char* LOG_FILE = "logs/securevault.log";

    // Internal Helpers
    static void log(std::string_view level, std::string_view message);
    static void ensureLogDirectoryExists();
    [[nodiscard]] static std::string getCurrentTimestamp();
};

} // namespace securevault