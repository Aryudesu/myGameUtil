#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace mygame::file {

inline bool Exists(const std::filesystem::path& path) {
    std::error_code ec;
    return std::filesystem::exists(path, ec);
}

inline bool IsFile(const std::filesystem::path& path) {
    std::error_code ec;
    return std::filesystem::is_regular_file(path, ec);
}

inline bool IsDirectory(const std::filesystem::path& path) {
    std::error_code ec;
    return std::filesystem::is_directory(path, ec);
}

inline bool EnsureDirectory(const std::filesystem::path& path) {
    if (path.empty()) return true;
    std::error_code ec;
    if (std::filesystem::exists(path, ec)) return std::filesystem::is_directory(path, ec);
    return std::filesystem::create_directories(path, ec);
}

inline bool EnsureParentDirectory(const std::filesystem::path& filePath) {
    return EnsureDirectory(filePath.parent_path());
}

inline std::optional<std::string> ReadAllText(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) return std::nullopt;
    std::ostringstream stream;
    stream << input.rdbuf();
    if (!input.good() && !input.eof()) return std::nullopt;
    return stream.str();
}

inline std::optional<std::vector<unsigned char>> ReadAllBytes(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input) return std::nullopt;

    const auto size = input.tellg();
    if (size < 0) return std::nullopt;
    input.seekg(0, std::ios::beg);

    std::vector<unsigned char> data(static_cast<std::size_t>(size));
    if (!data.empty()) {
        input.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(data.size()));
        if (!input) return std::nullopt;
    }
    return data;
}

inline bool WriteAllText(const std::filesystem::path& path, std::string_view text,
                         bool createParent = true) {
    if (createParent && !EnsureParentDirectory(path)) return false;
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) return false;
    output.write(text.data(), static_cast<std::streamsize>(text.size()));
    return static_cast<bool>(output);
}

inline bool WriteAllBytes(const std::filesystem::path& path,
                          const std::vector<unsigned char>& data,
                          bool createParent = true) {
    if (createParent && !EnsureParentDirectory(path)) return false;
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) return false;
    if (!data.empty()) {
        output.write(reinterpret_cast<const char*>(data.data()),
                     static_cast<std::streamsize>(data.size()));
    }
    return static_cast<bool>(output);
}

inline std::string Extension(const std::filesystem::path& path) {
    return path.extension().string();
}

inline std::string FileName(const std::filesystem::path& path) {
    return path.filename().string();
}

inline std::filesystem::path Join(const std::filesystem::path& left,
                                  const std::filesystem::path& right) {
    return left / right;
}

} // namespace mygame::file
