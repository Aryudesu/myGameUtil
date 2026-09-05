#pragma once

#include <mygame/encoding/Base64.h>

#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

namespace mygame::save {

// Lightweight reversible obfuscation for game save/config data.
// This is NOT cryptographic encryption and must not be used for secrets.
class SaveData {
public:
    static std::string Encode(std::string_view plainText, std::string_view key = {}) {
        const std::uint32_t checksum = Fnv1a(plainText);
        const std::string obfuscated = XorStream(plainText, SeedFromKey(key));

        std::ostringstream stream;
        stream << "MGU1:"
               << std::hex << std::setw(8) << std::setfill('0') << checksum
               << ':' << encoding::Base64Encode(obfuscated);
        return stream.str();
    }

    static std::optional<std::string> Decode(std::string_view encoded, std::string_view key = {}) {
        constexpr std::string_view prefix = "MGU1:";
        if (encoded.substr(0, prefix.size()) != prefix) return std::nullopt;

        const std::size_t separator = encoded.find(':', prefix.size());
        if (separator == std::string_view::npos) return std::nullopt;

        const auto checksumText = encoded.substr(prefix.size(), separator - prefix.size());
        if (checksumText.size() != 8) return std::nullopt;

        std::uint32_t expectedChecksum = 0;
        if (!ParseHex32(checksumText, expectedChecksum)) return std::nullopt;

        const auto decoded = encoding::Base64Decode(encoded.substr(separator + 1));
        if (!decoded) return std::nullopt;

        std::string plainText = XorStream(*decoded, SeedFromKey(key));
        if (Fnv1a(plainText) != expectedChecksum) return std::nullopt;
        return plainText;
    }

    static bool SaveToFile(const std::string& fileName,
                           std::string_view plainText,
                           std::string_view key = {}) {
        std::ofstream output(fileName, std::ios::binary | std::ios::trunc);
        if (!output) return false;
        const std::string encoded = Encode(plainText, key);
        output.write(encoded.data(), static_cast<std::streamsize>(encoded.size()));
        return static_cast<bool>(output);
    }

    static std::optional<std::string> LoadFromFile(const std::string& fileName,
                                                   std::string_view key = {}) {
        std::ifstream input(fileName, std::ios::binary);
        if (!input) return std::nullopt;
        const std::string encoded((std::istreambuf_iterator<char>(input)),
                                  std::istreambuf_iterator<char>());
        return Decode(encoded, key);
    }

private:
    static std::uint32_t Fnv1a(std::string_view data) {
        std::uint32_t hash = 2166136261u;
        for (unsigned char byte : data) {
            hash ^= byte;
            hash *= 16777619u;
        }
        return hash;
    }

    static std::uint32_t SeedFromKey(std::string_view key) {
        std::uint32_t seed = Fnv1a(key);
        return seed == 0 ? 0x9E3779B9u : seed;
    }

    static std::uint32_t Next(std::uint32_t& state) {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        return state;
    }

    static std::string XorStream(std::string_view input, std::uint32_t seed) {
        std::string output(input);
        std::uint32_t state = seed;
        std::uint32_t random = 0;
        int remaining = 0;

        for (char& ch : output) {
            if (remaining == 0) {
                random = Next(state);
                remaining = 4;
            }
            ch = static_cast<char>(static_cast<unsigned char>(ch) ^
                                   static_cast<unsigned char>(random & 0xFFu));
            random >>= 8;
            --remaining;
        }
        return output;
    }

    static bool ParseHex32(std::string_view text, std::uint32_t& value) {
        value = 0;
        for (char c : text) {
            std::uint32_t digit = 0;
            if (c >= '0' && c <= '9') digit = static_cast<std::uint32_t>(c - '0');
            else if (c >= 'a' && c <= 'f') digit = static_cast<std::uint32_t>(c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') digit = static_cast<std::uint32_t>(c - 'A' + 10);
            else return false;
            value = (value << 4) | digit;
        }
        return true;
    }
};

} // namespace mygame::save
