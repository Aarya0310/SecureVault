#include "../include/FileManager.h"
#include "../include/Logger.h"

#include <fstream>
#include <stdexcept>

namespace securevault {

namespace fs = std::filesystem;

std::vector<uint8_t> FileManager::readBinaryFile(const fs::path& filePath) {
    if (!fs::exists(filePath)) {
        Logger::logError("File read failed: File does not exist - " + filePath.string());
        throw std::runtime_error("File not found.");
    }

    if (!fs::is_regular_file(filePath)) {
        Logger::logError("File read failed: Not a regular file - " + filePath.string());
        throw std::runtime_error("Path is not a regular file.");
    }

    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        Logger::logError("File read failed: Cannot open file - " + filePath.string());
        throw std::runtime_error("Unable to open file for reading.");
    }

    // Seek to the end to determine the exact file size, then pre-allocate the vector
    // This is significantly faster than dynamically resizing the vector during a read
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        file.close();
        Logger::logInfo("Successfully read binary file: " + filePath.string());
        return buffer;
    } else {
        file.close();
        Logger::logError("File read failed: I/O error during read - " + filePath.string());
        throw std::runtime_error("Error reading file content.");
    }
}

bool FileManager::writeBinaryFile(const fs::path& filePath, const std::vector<uint8_t>& data) {
    // std::ios::trunc ensures that if the file exists, it is overwritten cleanly
    std::ofstream file(filePath, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        Logger::logError("File write failed: Cannot open file - " + filePath.string());
        return false;
    }

    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    file.close();

    Logger::logInfo("Successfully wrote binary file: " + filePath.string());
    return true;
}

bool FileManager::deleteFile(const fs::path& filePath) {
    if (fs::exists(filePath)) {
        bool result = fs::remove(filePath);
        if (result) {
            Logger::logInfo("Successfully deleted file: " + filePath.string());
        } else {
            Logger::logError("File deletion failed: " + filePath.string());
        }
        return result;
    }
    return false;
}

bool FileManager::fileExists(const fs::path& filePath) {
    return fs::exists(filePath);
}

size_t FileManager::getFileSize(const fs::path& filePath) {
    if (fs::exists(filePath) && fs::is_regular_file(filePath)) {
        return fs::file_size(filePath);
    }
    return 0;
}

std::string FileManager::getFileExtension(const fs::path& filePath) {
    if (fs::exists(filePath) && filePath.has_extension()) {
        return filePath.extension().string();
    }
    return "";
}

} // namespace securevault