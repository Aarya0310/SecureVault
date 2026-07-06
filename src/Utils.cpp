#include "../include/Utils.h"

#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

// Cross-platform headers for secure password input
#ifdef _WIN32
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif

namespace securevault::utils {

void clearScreen() {
    // ANSI escape code to clear the screen and move the cursor to the top-left (home) position
    std::cout << "\033[2J\033[1;1H";
}

void pause() {
    std::cout << "\nPress [Enter] to continue...";
    std::cin.get();
}

void printHeader(std::string_view subtitle) {
    clearScreen();
    std::cout << "====================================================\n";
    std::cout << "               SecureVault++                        \n";
    std::cout << "====================================================\n";
    if (!subtitle.empty()) {
        std::cout << "  " << subtitle << "\n";
        std::cout << "====================================================\n";
    }
    std::cout << "\n";
}

void printMenuHeader(std::string_view title) {
    std::cout << "\n--- " << title << " ---\n\n";
}

void showProgressBar(size_t current, size_t total, std::string_view prefix) {
    if (total == 0) return;

    const int barWidth = 40;
    float progress = static_cast<float>(current) / static_cast<float>(total);
    int pos = static_cast<int>(barWidth * progress);

    std::cout << prefix << " [";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();

    // If complete, move to the next line to prevent overwriting
    if (current == total) {
        std::cout << "\n";
    }
}

std::string readHiddenPassword() {
    std::string password;
    
#ifdef _WIN32
    char ch;
    while ((ch = _getch()) != '\r') { // Enter key is \r on Windows
        if (ch == '\b') { // Backspace
            if (!password.empty()) {
                std::cout << "\b \b";
                password.pop_back();
            }
        } else {
            password.push_back(ch);
            std::cout << '*';
        }
    }
    std::cout << '\n';
#else
    // POSIX compliant secure input (Linux/macOS)
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~ECHO; // Disable echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::getline(std::cin, password);

    // Restore terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << '\n';
#endif

    return password;
}

} // namespace securevault::utils