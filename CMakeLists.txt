cmake_minimum_required(VERSION 3.10)

# Project Name and Version
project(SecureVault VERSION 1.0.0 LANGUAGES CXX)

# Enforce C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Specify all source files
set(SOURCES
    src/main.cpp
    src/User.cpp
    src/Encryption.cpp
    src/Vault.cpp
    src/FileManager.cpp
    src/Logger.cpp
    src/Utils.cpp
)

# Create the executable
add_executable(SecureVault ${SOURCES})

# Include directories
target_include_directories(SecureVault PRIVATE include)

# Find and link threads (Required for std::mutex in Logger)
find_package(Threads REQUIRED)
target_link_libraries(SecureVault PRIVATE Threads::Threads)