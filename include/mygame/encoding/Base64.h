#pragma once

#include <array>
#include <cctype>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace mygame::encoding {

inline std::string Base64Encode(std::string_view input) {
    static constexpr char table[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string output;
    output.reserve(((input.size() + 2) / 3) * 4);

    std::size_t i = 0;
    while (i + 3 <= input.size()) {
        const std::uint32_t block =
            (static_cast<std::uint32_t>(static_cast<unsigned char>(input[i])) << 16) |
            (static_cast<std::uint32_t>(static_cast<unsigned char>(input[i + 1])) << 8) |
            static_cast<std::uint32_t>(static_cast<unsigned char>(input[i + 2]));
        output.push_back(table[(block >> 18) & 0x3F]);
        output.push_back(table[(block >> 12) & 0x3F]);
        output.push_back(table[(block >> 6) & 0x3F]);
        output.push_back(table[block & 0x3F]);
        i += 3;
    }

    const std::size_t remaining = input.size() - i;
    if (remaining == 1) {
        const std::uint32_t block =
            static_cast<std::uint32_t>(static_cast<unsigned char>(input[i])) << 16;
        output.push_back(table[(block >> 18) & 0x3F]);
        output.push_back(table[(block >> 12) & 0x3F]);
        output.push_back('=');
        output.push_back('=');
    }
    else if (remaining == 2) {
        const std::uint32_t block =
            (static_cast<std::uint32_t>(static_cast<unsigned char>(input[i])) << 16) |
            (static_cast<std::uint32_t>(static_cast<unsigned char>(input[i + 1])) << 8);
        output.push_back(table[(block >> 18) & 0x3F]);
        output.push_back(table[(block >> 12) & 0x3F]);
        output.push_back(table[(block >> 6) & 0x3F]);
        output.push_back('=');
    }

    return output;
}

inline std::optional<std::string> Base64Decode(std::string_view input) {
    if (input.size() % 4 != 0) return std::nullopt;

    std::array<int, 256> reverse{};
    reverse.fill(-1);
    constexpr std::string_view table =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    for (std::size_t i = 0; i < table.size(); ++i) {
        reverse[static_cast<unsigned char>(table[i])] = static_cast<int>(i);
    }

    std::string output;
    output.reserve((input.size() / 4) * 3);

    for (std::size_t i = 0; i < input.size(); i += 4) {
        const char c0 = input[i];
        const char c1 = input[i + 1];
        const char c2 = input[i + 2];
        const char c3 = input[i + 3];

        if (c0 == '=' || c1 == '=') return std::nullopt;
        const int v0 = reverse[static_cast<unsigned char>(c0)];
        const int v1 = reverse[static_cast<unsigned char>(c1)];
        if (v0 < 0 || v1 < 0) return std::nullopt;

        const bool pad2 = c2 == '=';
        const bool pad3 = c3 == '=';
        if (pad2 && !pad3) return std::nullopt;
        if ((pad2 || pad3) && i + 4 != input.size()) return std::nullopt;

        const int v2 = pad2 ? 0 : reverse[static_cast<unsigned char>(c2)];
        const int v3 = pad3 ? 0 : reverse[static_cast<unsigned char>(c3)];
        if (v2 < 0 || v3 < 0) return std::nullopt;

        const std::uint32_t block =
            (static_cast<std::uint32_t>(v0) << 18) |
            (static_cast<std::uint32_t>(v1) << 12) |
            (static_cast<std::uint32_t>(v2) << 6) |
            static_cast<std::uint32_t>(v3);

        output.push_back(static_cast<char>((block >> 16) & 0xFF));
        if (!pad2) output.push_back(static_cast<char>((block >> 8) & 0xFF));
        if (!pad3) output.push_back(static_cast<char>(block & 0xFF));
    }

    return output;
}

} // namespace mygame::encoding
