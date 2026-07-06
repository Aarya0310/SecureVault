#include "../include/Encryption.h"
#include <stdexcept>

namespace securevault {

Encryption::Encryption(std::string_view key) : m_key(key) {
    if (m_key.empty()) {
        throw std::invalid_argument("Security Error: Encryption key cannot be empty.");
    }
}

std::vector<uint8_t> Encryption::processXOR(const std::vector<uint8_t>& data) const {
    if (data.empty()) {
        return {};
    }

    // Pre-allocate memory to prevent expensive reallocations during processing
    std::vector<uint8_t> processedData;
    processedData.reserve(data.size());

    size_t keyLength = m_key.length();
    
    for (size_t i = 0; i < data.size(); ++i) {
        // XOR each byte of data with a byte of the key, looping safely using modulo
        processedData.push_back(data[i] ^ m_key[i % keyLength]);
    }

    return processedData;
}

std::vector<uint8_t> Encryption::encrypt(const std::vector<uint8_t>& data) const {
    // Version 1: XOR implementation. 
    // To upgrade to AES-256, simply replace processXOR() with a block cipher call here.
    return processXOR(data);
}

std::vector<uint8_t> Encryption::decrypt(const std::vector<uint8_t>& data) const {
    // Version 1: XOR implementation.
    // XOR is symmetric, meaning encryption and decryption are the exact same mathematical operation.
    // When upgrading to AES-256, this will need to call your specific AES decryption logic.
    return processXOR(data);
}

} // namespace securevault