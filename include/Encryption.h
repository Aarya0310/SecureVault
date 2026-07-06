#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <cstdint>

namespace securevault {

class Encryption {
public:
    // Force explicit initialization with a key
    explicit Encryption(std::string_view key);
    ~Encryption() = default;

    // Delete default, copy, and move semantics to prevent accidental key duplication in memory
    Encryption() = delete;
    Encryption(const Encryption&) = delete;
    Encryption& operator=(const Encryption&) = delete;
    Encryption(Encryption&&) = delete;
    Encryption& operator=(Encryption&&) = delete;

    // Core Public Interface
    [[nodiscard]] std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data) const;
    [[nodiscard]] std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data) const;

private:
    std::string m_key;
    
    // Internal cryptographic engine (V1: XOR)
    [[nodiscard]] std::vector<uint8_t> processXOR(const std::vector<uint8_t>& data) const;
};

} // namespace securevault