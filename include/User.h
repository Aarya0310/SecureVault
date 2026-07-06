#pragma once

#include <string>
#include <string_view>

namespace securevault {

class User {
public:
    User();
    ~User() = default;

    // Delete copy and move semantics to prevent credential duplication in memory
    User(const User&) = delete;
    User& operator=(const User&) = delete;
    User(User&&) = delete;
    User& operator=(User&&) = delete;

    // Core Authentication
    bool registerUser(std::string_view username, std::string_view password);
    bool login(std::string_view username, std::string_view password);
    void logout();

    // Account Management
    bool changePassword(std::string_view oldPassword, std::string_view newPassword);
    
    // Utilities
    static bool isPasswordStrong(std::string_view password);

    // Getters
    [[nodiscard]] bool isAuthenticated() const;
    [[nodiscard]] std::string getUsername() const;
    [[nodiscard]] std::string getEncryptionKey() const;

private:
    std::string m_username;
    std::string m_encryptionKey;
    bool m_isAuthenticated;
    int m_loginAttempts;
    
    static constexpr int MAX_LOGIN_ATTEMPTS = 3;
    static constexpr const char* USERS_DIR = "users/";

    // Internal Helpers
    [[nodiscard]] std::string hashPassword(std::string_view password) const;
    [[nodiscard]] std::string getUserFilePath(std::string_view username) const;
    void ensureUserDirectoryExists() const;
};

} // namespace securevault