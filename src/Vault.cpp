#include "../include/Vault.h"
#include "../include/Logger.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <iomanip>

namespace securevault {

namespace fs = std::filesystem;

Vault::Vault(const User& user) 
    : m_user(user), 
      m_encryption(user.getEncryptionKey()) {
    
    if (!m_user.isAuthenticated()) {
        Logger::logError("Vault Access Denied: User is not authenticated.");
        throw std::runtime_error("Cannot instantiate Vault: User not authenticated.");
    }

    ensureDirectoriesExist();
    Logger::logInfo("Vault instantiated securely for user: " + m_user.getUsername());
}

std::string Vault::getUserVaultDirectory() const {
    return "vault/" + m_user.getUsername() + "/";
}

std::string Vault::getUserNotesDirectory() const {
    return getUserVaultDirectory() + "notes/";
}

std::string Vault::getUserFilesDirectory() const {
    return getUserVaultDirectory() + "files/";
}

void Vault::ensureDirectoriesExist() const {
    if (!fs::exists("vault")) fs::create_directory("vault");
    if (!fs::exists(getUserVaultDirectory())) fs::create_directory(getUserVaultDirectory());
    if (!fs::exists(getUserNotesDirectory())) fs::create_directory(getUserNotesDirectory());
    if (!fs::exists(getUserFilesDirectory())) fs::create_directory(getUserFilesDirectory());
}

std::string Vault::getNoteFilePath(std::string_view title) const {
    return getUserNotesDirectory() + std::string(title) + ".enc";
}

std::vector<uint8_t> Vault::stringToBytes(std::string_view str) const {
    return std::vector<uint8_t>(str.begin(), str.end());
}

std::string Vault::bytesToString(const std::vector<uint8_t>& bytes) const {
    return std::string(bytes.begin(), bytes.end());
}

// Helper to safely convert C++17 std::filesystem::file_time_type to a readable string
std::string Vault::formatTime(const fs::file_time_type& ftime) const {
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(
        ftime - fs::file_time_type::clock::now() + system_clock::now()
    );
    std::time_t cftime = system_clock::to_time_t(sctp);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&cftime), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

bool Vault::createNote(std::string_view title, std::string_view content) const {
    std::string filePath = getNoteFilePath(title);
    if (fs::exists(filePath)) {
        Logger::logWarning("Note creation failed: Note already exists - " + std::string(title));
        return false;
    }

    std::vector<uint8_t> plainBytes = stringToBytes(content);
    std::vector<uint8_t> cipherBytes = m_encryption.encrypt(plainBytes);

    std::ofstream noteFile(filePath, std::ios::out | std::ios::binary);
    if (!noteFile.is_open()) {
        Logger::logError("Note creation failed: File I/O error - " + std::string(title));
        throw std::runtime_error("Failed to open file for writing note.");
    }

    noteFile.write(reinterpret_cast<const char*>(cipherBytes.data()), cipherBytes.size());
    noteFile.close();

    Logger::logInfo("Secure Note created successfully: " + std::string(title));
    return true;
}

std::optional<std::string> Vault::viewNote(std::string_view title) const {
    std::string filePath = getNoteFilePath(title);
    if (!fs::exists(filePath)) {
        Logger::logWarning("Note viewing failed: Note not found - " + std::string(title));
        return std::nullopt;
    }

    std::ifstream noteFile(filePath, std::ios::in | std::ios::binary);
    if (!noteFile.is_open()) {
        Logger::logError("Note viewing failed: File I/O error - " + std::string(title));
        return std::nullopt;
    }

    std::vector<uint8_t> cipherBytes((std::istreambuf_iterator<char>(noteFile)),
                                      std::istreambuf_iterator<char>());
    noteFile.close();

    std::vector<uint8_t> plainBytes = m_encryption.decrypt(cipherBytes);
    return bytesToString(plainBytes);
}

bool Vault::editNote(std::string_view title, std::string_view newContent) const {
    if (!fs::exists(getNoteFilePath(title))) {
        Logger::logWarning("Note editing failed: Note not found - " + std::string(title));
        return false;
    }

    // Overwrite the existing file by passing std::ios::trunc (default with ios::out)
    std::vector<uint8_t> plainBytes = stringToBytes(newContent);
    std::vector<uint8_t> cipherBytes = m_encryption.encrypt(plainBytes);

    std::ofstream noteFile(getNoteFilePath(title), std::ios::out | std::ios::binary | std::ios::trunc);
    if (!noteFile.is_open()) {
        Logger::logError("Note editing failed: File I/O error - " + std::string(title));
        return false;
    }

    noteFile.write(reinterpret_cast<const char*>(cipherBytes.data()), cipherBytes.size());
    noteFile.close();

    Logger::logInfo("Secure Note edited successfully: " + std::string(title));
    return true;
}

bool Vault::deleteNote(std::string_view title) const {
    std::string filePath = getNoteFilePath(title);
    if (fs::exists(filePath)) {
        fs::remove(filePath);
        Logger::logInfo("Secure Note deleted: " + std::string(title));
        return true;
    }
    Logger::logWarning("Note deletion failed: Note not found - " + std::string(title));
    return false;
}

std::vector<std::string> Vault::searchNotes(std::string_view keyword) const {
    std::vector<std::string> matchedNotes;
    std::string searchWord(keyword);

    // Convert search keyword to lowercase for case-insensitive search
    std::transform(searchWord.begin(), searchWord.end(), searchWord.begin(), ::tolower);

    for (const auto& entry : fs::directory_iterator(getUserNotesDirectory())) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().stem().string();
            auto contentOpt = viewNote(filename);
            
            if (contentOpt) {
                std::string content = *contentOpt;
                std::transform(content.begin(), content.end(), content.begin(), ::tolower);

                if (content.find(searchWord) != std::string::npos) {
                    matchedNotes.push_back(filename);
                }
            }
        }
    }
    
    Logger::logInfo("Performed search in notes for keyword: " + std::string(keyword));
    return matchedNotes;
}

std::vector<FileMetadata> Vault::listSecureNotes() const {
    std::vector<FileMetadata> notes;
    for (const auto& entry : fs::directory_iterator(getUserNotesDirectory())) {
        if (entry.is_regular_file()) {
            notes.push_back({
                entry.path().stem().string(), // Note title without .enc extension
                fs::file_size(entry),
                formatTime(fs::last_write_time(entry))
            });
        }
    }
    return notes;
}

std::vector<FileMetadata> Vault::listEncryptedFiles() const {
    std::vector<FileMetadata> files;
    for (const auto& entry : fs::directory_iterator(getUserFilesDirectory())) {
        if (entry.is_regular_file()) {
            files.push_back({
                entry.path().filename().string(), // Keep extension for files
                fs::file_size(entry),
                formatTime(fs::last_write_time(entry))
            });
        }
    }
    return files;
}

} // namespace securevault