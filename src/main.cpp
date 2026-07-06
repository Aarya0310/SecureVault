#include "../include/User.h"
#include "../include/Vault.h"
#include "../include/Encryption.h"
#include "../include/FileManager.h"
#include "../include/Logger.h"
#include "../include/Utils.h"

#include <iostream>
#include <string>
#include <limits>
#include <filesystem>
#include <stdexcept>

using namespace securevault;
namespace fs = std::filesystem;

// Helper to clear input buffer when user enters invalid characters
void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// Sub-menu for Secure Notes
void handleSecureNotesMenu(const Vault& vault) {
    while (true) {
        utils::printHeader("Secure Notes");
        std::cout << "1. Create Note\n";
        std::cout << "2. View Note\n";
        std::cout << "3. Edit Note\n";
        std::cout << "4. Delete Note\n";
        std::cout << "5. Back to Main Menu\n\n";
        std::cout << "Select an option: ";

        int choice;
        if (!(std::cin >> choice)) {
            clearInputBuffer();
            continue;
        }
        clearInputBuffer(); // Clear the newline

        if (choice == 5) break;

        std::string title, content;
        switch (choice) {
            case 1:
                std::cout << "\nEnter Note Title: ";
                std::getline(std::cin, title);
                std::cout << "Enter Note Content: ";
                std::getline(std::cin, content);
                if (vault.createNote(title, content)) {
                    std::cout << "\n[SUCCESS] Note created.\n";
                } else {
                    std::cout << "\n[FAILED] Note creation failed.\n";
                }
                break;
            case 2:
                std::cout << "\nEnter Note Title to View: ";
                std::getline(std::cin, title);
                if (auto noteContent = vault.viewNote(title)) {
                    std::cout << "\n--- " << title << " ---\n";
                    std::cout << *noteContent << "\n-------------------\n";
                } else {
                    std::cout << "\n[FAILED] Note not found.\n";
                }
                break;
            case 3:
                std::cout << "\nEnter Note Title to Edit: ";
                std::getline(std::cin, title);
                std::cout << "Enter New Content (will overwrite): ";
                std::getline(std::cin, content);
                if (vault.editNote(title, content)) {
                    std::cout << "\n[SUCCESS] Note updated.\n";
                } else {
                    std::cout << "\n[FAILED] Note update failed.\n";
                }
                break;
            case 4:
                std::cout << "\nEnter Note Title to Delete: ";
                std::getline(std::cin, title);
                if (vault.deleteNote(title)) {
                    std::cout << "\n[SUCCESS] Note deleted.\n";
                } else {
                    std::cout << "\n[FAILED] Note deletion failed.\n";
                }
                break;
            default:
                std::cout << "\nInvalid choice.\n";
        }
        utils::pause();
    }
}

// Main authenticated application loop
void runApplication(User& user) {
    Vault vault(user);
    Encryption cipher(user.getEncryptionKey());

    while (user.isAuthenticated()) {
        utils::printHeader();
        utils::printMenuHeader("Welcome " + user.getUsername());
        
        std::cout << "1. Secure Notes\n";
        std::cout << "2. Encrypt File\n";
        std::cout << "3. Decrypt File\n";
        std::cout << "4. View Vault\n";
        std::cout << "5. Search Notes\n";
        std::cout << "6. Change Password\n";
        std::cout << "7. Activity Logs\n";
        std::cout << "8. Logout\n\n";
        std::cout << "Select an option: ";

        int choice;
        if (!(std::cin >> choice)) {
            clearInputBuffer();
            continue;
        }
        clearInputBuffer(); 

        try {
            switch (choice) {
                case 1:
                    handleSecureNotesMenu(vault);
                    break;
                case 2: {
                    std::cout << "\nEnter full path of the file to encrypt (e.g., document.pdf): ";
                    std::string inputPath;
                    std::getline(std::cin, inputPath);
                    
                    if (!FileManager::fileExists(inputPath)) {
                        std::cout << "[ERROR] File not found.\n";
                    } else {
                        fs::path p(inputPath);
                        std::string filename = p.filename().string();
                        std::string outputPath = vault.getUserFilesDirectory() + filename + ".enc";

                        size_t fileSize = FileManager::getFileSize(inputPath);
                        std::cout << "Reading file (" << fileSize << " bytes)...\n";
                        
                        auto plainBytes = FileManager::readBinaryFile(inputPath);
                        utils::showProgressBar(50, 100, "Encrypting");
                        
                        auto cipherBytes = cipher.encrypt(plainBytes);
                        
                        utils::showProgressBar(100, 100, "Encrypting");
                        if (FileManager::writeBinaryFile(outputPath, cipherBytes)) {
                            Logger::logInfo("Encrypted external file: " + filename);
                            std::cout << "[SUCCESS] File encrypted and stored in vault.\n";
                        }
                    }
                    utils::pause();
                    break;
                }
                case 3: {
                    std::cout << "\nEnter filename in vault to decrypt (e.g., document.pdf.enc): ";
                    std::string filename;
                    std::getline(std::cin, filename);
                    
                    std::string inputPath = vault.getUserFilesDirectory() + filename;
                    if (!FileManager::fileExists(inputPath)) {
                        std::cout << "[ERROR] Encrypted file not found in vault.\n";
                    } else {
                        // Create a decrypted output folder in the root directory
                        if (!fs::exists("decrypted_files")) fs::create_directory("decrypted_files");
                        
                        // Strip the .enc extension for the output name
                        std::string outName = filename;
                        if (outName.length() >= 4 && outName.substr(outName.length() - 4) == ".enc") {
                            outName = outName.substr(0, outName.length() - 4);
                        }
                        std::string outputPath = "decrypted_files/" + outName;

                        auto cipherBytes = FileManager::readBinaryFile(inputPath);
                        utils::showProgressBar(50, 100, "Decrypting");
                        
                        auto plainBytes = cipher.decrypt(cipherBytes);
                        
                        utils::showProgressBar(100, 100, "Decrypting");
                        if (FileManager::writeBinaryFile(outputPath, plainBytes)) {
                            Logger::logInfo("Decrypted file exported: " + outName);
                            std::cout << "[SUCCESS] File decrypted and exported to 'decrypted_files/' folder.\n";
                        }
                    }
                    utils::pause();
                    break;
                }
                case 4: {
                    utils::printHeader("Vault Contents");
                    
                    auto files = vault.listEncryptedFiles();
                    auto notes = vault.listSecureNotes();

                    std::cout << "--- Encrypted Files ---\n";
                    if (files.empty()) std::cout << "(Empty)\n";
                    for (const auto& f : files) {
                        std::cout << "- " << f.filename << " | " << f.sizeBytes << " bytes | " << f.creationDate << "\n";
                    }

                    std::cout << "\n--- Secure Notes ---\n";
                    if (notes.empty()) std::cout << "(Empty)\n";
                    for (const auto& n : notes) {
                        std::cout << "- " << n.filename << " | " << n.sizeBytes << " bytes | " << n.creationDate << "\n";
                    }
                    utils::pause();
                    break;
                }
                case 5: {
                    std::cout << "\nEnter search keyword: ";
                    std::string keyword;
                    std::getline(std::cin, keyword);
                    
                    auto results = vault.searchNotes(keyword);
                    std::cout << "\n--- Search Results ---\n";
                    if (results.empty()) {
                        std::cout << "No matching notes found.\n";
                    } else {
                        for (const auto& title : results) {
                            std::cout << "- " << title << "\n";
                        }
                    }
                    utils::pause();
                    break;
                }
                case 6: {
                    std::cout << "\nEnter Current Password: ";
                    std::string oldPass = utils::readHiddenPassword();
                    std::cout << "Enter New Password: ";
                    std::string newPass = utils::readHiddenPassword();
                    
                    if (user.changePassword(oldPass, newPass)) {
                        std::cout << "[SUCCESS] Password updated.\n";
                    } else {
                        std::cout << "[ERROR] Password update failed. Ensure it meets strength requirements.\n";
                    }
                    utils::pause();
                    break;
                }
                case 7: {
                    utils::printHeader("Activity Logs");
                    if (FileManager::fileExists("logs/securevault.log")) {
                        auto logBytes = FileManager::readBinaryFile("logs/securevault.log");
                        std::string logContent(logBytes.begin(), logBytes.end());
                        std::cout << logContent << "\n";
                    } else {
                        std::cout << "No logs found.\n";
                    }
                    utils::pause();
                    break;
                }
                case 8:
                    user.logout();
                    std::cout << "\nLogging out...\n";
                    utils::pause();
                    break;
                default:
                    std::cout << "\nInvalid choice.\n";
                    utils::pause();
            }
        } catch (const std::exception& e) {
            std::cout << "\n[CRITICAL ERROR] " << e.what() << "\n";
            Logger::logError(std::string("Unhandled exception in UI: ") + e.what());
            utils::pause();
        }
    }
}

// Entry point and pre-login menu
int main() {
    User user;
    
    while (true) {
        utils::printHeader();
        std::cout << "1. Register\n";
        std::cout << "2. Login\n";
        std::cout << "3. Exit\n\n";
        std::cout << "Select an option: ";

        int choice;
        if (!(std::cin >> choice)) {
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        if (choice == 3) {
            std::cout << "\nExiting SecureVault++...\n";
            break;
        }

        std::string username, password;
        switch (choice) {
            case 1:
                std::cout << "\n--- Registration ---\n";
                std::cout << "Enter Username: ";
                std::getline(std::cin, username);
                std::cout << "Enter Password (min 8 chars, Upper, Lower, Digit, Special): ";
                password = utils::readHiddenPassword();
                
                if (user.registerUser(username, password)) {
                    std::cout << "[SUCCESS] Account created successfully.\n";
                } else {
                    std::cout << "[FAILED] Registration failed. Check password strength or if user exists.\n";
                }
                utils::pause();
                break;

            case 2:
                std::cout << "\n--- Login ---\n";
                std::cout << "Enter Username: ";
                std::getline(std::cin, username);
                std::cout << "Enter Password: ";
                password = utils::readHiddenPassword();

                if (user.login(username, password)) {
                    runApplication(user); // Transition to vault state
                } else {
                    std::cout << "[FAILED] Invalid credentials or account locked.\n";
                    utils::pause();
                }
                break;

            default:
                std::cout << "\nInvalid choice.\n";
                utils::pause();
        }
    }

    return 0;
}