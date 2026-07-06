#pragma once

#include <string>
#include <string_view>

namespace securevault::utils {

    // Console Control
    void clearScreen();
    void pause();

    // UI Formatting
    void printHeader(std::string_view subtitle = "");
    void printMenuHeader(std::string_view title);
    
    // Progress Indication
    void showProgressBar(size_t current, size_t total, std::string_view prefix = "Processing");

    // Secure Input Handling
    [[nodiscard]] std::string readHiddenPassword();

} // namespace securevault::utils