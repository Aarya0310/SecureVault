#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <cstdint>

namespace securevault {

class FileManager {
public:
    // Delete constructor: This is a purely static utility class
    FileManager() = delete;

    // Core Binary I/O
    [[nodiscard]] static std::vector<uint8_t> readBinaryFile(const std::filesystem::path& filePath);
    static bool writeBinaryFile(const std::filesystem::path& filePath, const std::vector<uint8_t>& data);
    
    // File Utilities
    static bool deleteFile(const std::filesystem::path& filePath);
    [[nodiscard]] static bool fileExists(const std::filesystem::path& filePath);
    [[nodiscard]] static size_t getFileSize(const std::filesystem::path& filePath);
    [[nodiscard]] static std::string getFileExtension(const std::filesystem::path& filePath);
};

} // namespace securevault