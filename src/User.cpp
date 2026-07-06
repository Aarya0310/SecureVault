#include "../include/User.h"
#include "../include/Logger.h"

#include <filesystem>
#include <fstream>
#include <functional>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <stdexcept>

namespace securevault {

namespace fs = std::filesystem;

User::User() : m_isAuthenticated(false), m_loginAttempts(0) {
    ensureUserDirectoryExists();
}

void User::ensureUserDirectoryExists() const {
    if (!fs::exists(USERS_DIR)) {
        fs::create_directory(USERS_DIR);
    }
}

std::string User::getUserFilePath(std::string_view username) const {
    return std::string(USERS_DIR) + std::string(username) + ".dat";
}

std::string User::hashPassword(std::string_view password) const {
    // Note: std::hash is used here per requirements. 
    // In a production environment, Argon2 or bcrypt would be utilized.
    std::hash<std::string_view> hasher;
    size_t hashValue = hasher(password);
    
    // Convert the size_t hash to a hex string representation for safe file storage
    std::stringstream hexStream;
    hexStream << std::hex << std::setw(sizeof(size_t) * 2) << std::setfill('0') << hashValue;
    return hexStream.str();
}

bool User::isPasswordStrong(std::string_view password) {
    if (password.length() < 8) return false;

    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;
    bool hasSpecial = false;

    for (char ch : password) {
        if (std::isupper(ch)) hasUpper = true;
        else if (std::islower(ch)) hasLower = true;
        else if (std::isdigit(ch)) hasDigit = true;
        else if (std::ispunct(ch)) hasSpecial = true;
    }

    return hasUpper && hasLower && hasDigit && hasSpecial;
}

bool User::registerUser(std::string_view username, std::string_view password) {
    std::string filePath = getUserFilePath(username);

    if (fs::exists(filePath)) {
        Logger::logWarning("Registration failed: User already exists - " + std::string(username));
        return false;
    }

    if (!isPasswordStrong(password)) {
        Logger::logWarning("Registration failed: Weak password provided for - " + std::string(username));
        return false;
    }

    std::ofstream userFile(filePath, std::ios::out | std::ios::binary);
    if (!userFile.is_open()) {
        Logger::logError("Registration failed: Could not create file for - " + std::string(username));
        throw std::runtime_error("Unable to open user file for writing.");
    }

    std::string hashedPwd = hashPassword(password);
    userFile.write(hashedPwd.c_str(), hashedPwd.size());
    userFile.close();

    Logger::logInfo("Registration successful for user: " + std::string(username));
    return true;
}

bool User::login(std::string_view username, std::string_view password) {
    if (m_isAuthenticated) {
        return true; 
    }

    if (m_loginAttempts >= MAX_LOGIN_ATTEMPTS) {
        Logger::logWarning("Login locked due to maximum attempts reached for user: " + std::string(username));
        return false;
    }

    std::string filePath = getUserFilePath(username);
    if (!fs::exists(filePath)) {
        m_loginAttempts++;
        Logger::logWarning("Login failed: User not found - " + std::string(username));
        return false;
    }

    std::ifstream userFile(filePath, std::ios::in | std::ios::binary);
    if (!userFile.is_open()) {
        Logger::logError("Login failed: Could not read file for - " + std::string(username));
        throw std::runtime_error("Unable to open user file for reading.");
    }

    std::string storedHash;
    userFile >> storedHash;
    userFile.close();

    std::string inputHash = hashPassword(password);

    if (storedHash == inputHash) {
        m_isAuthenticated = true;
        m_username = username;
        
        // Derive a simple encryption key from the hash for V1 XOR encryption
        m_encryptionKey = inputHash; 
        m_loginAttempts = 0;
        
        Logger::logInfo("Login successful for user: " + std::string(username));
        return true;
    }

    m_loginAttempts++;
    Logger::logWarning("Login failed: Incorrect password for - " + std::string(username));
    return false;
}

void User::logout() {
    if (m_isAuthenticated) {
        Logger::logInfo("User logged out: " + m_username);
        
        // Securely wipe memory state
        m_isAuthenticated = false;
        m_username.clear();
        m_encryptionKey.assign(m_encryptionKey.size(), '\0');
        m_encryptionKey.clear();
        m_loginAttempts = 0;
    }
}

bool User::changePassword(std::string_view oldPassword, std::string_view newPassword) {
    if (!m_isAuthenticated) {
        return false;
    }

    std::string filePath = getUserFilePath(m_username);
    std::ifstream userFile(filePath, std::ios::in | std::ios::binary);
    
    std::string storedHash;
    if (userFile.is_open()) {
        userFile >> storedHash;
        userFile.close();
    }

    if (storedHash != hashPassword(oldPassword)) {
        Logger::logWarning("Password change failed: Incorrect old password for - " + m_username);
        return false;
    }

    if (!isPasswordStrong(newPassword)) {
        Logger::logWarning("Password change failed: Weak new password for - " + m_username);
        return false;
    }

    std::ofstream outFile(filePath, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!outFile.is_open()) {
        Logger::logError("Password change failed: Could not write to file for - " + m_username);
        throw std::runtime_error("Unable to open user file for writing.");
    }

    std::string newHash = hashPassword(newPassword);
    outFile.write(newHash.c_str(), newHash.size());
    outFile.close();

    m_encryptionKey = newHash; // Update active session key
    Logger::logInfo("Password successfully changed for user: " + m_username);
    
    return true;
}

bool User::isAuthenticated() const {
    return m_isAuthenticated;
}

std::string User::getUsername() const {
    return m_username;
}

std::string User::getEncryptionKey() const {
    return m_encryptionKey;
}

} // namespace securevault