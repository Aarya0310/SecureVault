#pragma once

#include "User.h"
#include "Encryption.h"

#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <cstdint>
#include <filesystem>

namespace securevault {

struct FileMetadata {
    std::string filename;
    size_t sizeBytes;
    std::string creationDate;
};

class Vault {
public:
    // Vault must be initialized with an authenticated user context
    explicit Vault(const User& user);
    ~Vault() = default;

    // Delete copy/move semantics to prevent cross-contamination of vault states
    Vault(const Vault&) = delete;
    Vault& operator=(const Vault&) = delete;
    Vault(Vault&&) = delete;
    Vault& operator=(Vault&&) = delete;

    // Secure Notes CRUD Operations
    bool createNote(std::string_view title, std::string_view content) const;
    [[nodiscard]] std::optional<std::string> viewNote(std::string_view title) const;
    bool editNote(std::string_view title, std::string_view newContent) const;
    bool deleteNote(std::string_view title) const;
    
    // Search within encrypted notes
    [[nodiscard]] std::vector<std::string> searchNotes(std::string_view keyword) const;

    // Vault Indexing and Metadata
    [[nodiscard]] std::vector<FileMetadata> listEncryptedFiles() const;
    [[nodiscard]] std::vector<FileMetadata> listSecureNotes() const;

    // Path Accessors (Used by FileManager in later phases)
    [[nodiscard]] std::string getUserVaultDirectory() const;
    [[nodiscard]] std::string getUserNotesDirectory() const;
    [[nodiscard]] std::string getUserFilesDirectory() const;

private:
    const User& m_user; 
    Encryption m_encryption;

    // Directory Management
    void ensureDirectoriesExist() const;
    [[nodiscard]] std::string getNoteFilePath(std::string_view title) const;

    // Internal data conversion helpers
    [[nodiscard]] std::vector<uint8_t> stringToBytes(std::string_view str) const;
    [[nodiscard]] std::string bytesToString(const std::vector<uint8_t>& bytes) const;
    [[nodiscard]] std::string formatTime(const std::filesystem::file_time_type& ftime) const;
};

} // namespace securevault