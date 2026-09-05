#pragma once

#include <algorithm>
#include <cctype>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace mygame {

class IniConfig {
public:
    IniConfig() = default;
    explicit IniConfig(const std::string& fileName) { Load(fileName); }

    bool Load(const std::string& fileName) {
        std::ifstream input(fileName);
        if (!input) return false;

        data_.clear();
        std::string section;
        std::string line;

        while (std::getline(input, line)) {
            line = Trim(line);
            if (line.empty() || line.front() == '#' || line.front() == ';') continue;

            const auto comment = line.find_first_of("#;");
            if (comment != std::string::npos) line = Trim(line.substr(0, comment));
            if (line.empty()) continue;

            if (line.front() == '>' ) {
                section = Trim(line.substr(1));
                continue;
            }
            if (line.size() >= 2 && line.front() == '[' && line.back() == ']') {
                section = Trim(line.substr(1, line.size() - 2));
                continue;
            }

            const auto equal = line.find('=');
            if (equal == std::string::npos) continue;

            const std::string key = Trim(line.substr(0, equal));
            const std::string value = Trim(line.substr(equal + 1));
            if (key.empty()) continue;

            data_[section][key] = SplitCsv(value);
        }
        return true;
    }

    void Clear() { data_.clear(); }

    bool HasSection(const std::string& section) const {
        return data_.find(section) != data_.end();
    }

    bool HasKey(const std::string& section, const std::string& key) const {
        const auto sec = data_.find(section);
        return sec != data_.end() && sec->second.find(key) != sec->second.end();
    }

    std::optional<std::vector<std::string>> TryGet(const std::string& section,
                                                   const std::string& key) const {
        const auto sec = data_.find(section);
        if (sec == data_.end()) return std::nullopt;
        const auto value = sec->second.find(key);
        if (value == sec->second.end()) return std::nullopt;
        return value->second;
    }

    std::string GetString(const std::string& section, const std::string& key,
                          const std::string& defaultValue = "") const {
        const auto value = TryGet(section, key);
        return value && !value->empty() ? value->front() : defaultValue;
    }

    int GetInt(const std::string& section, const std::string& key, int defaultValue = 0) const {
        try { return std::stoi(GetString(section, key)); }
        catch (...) { return defaultValue; }
    }

    float GetFloat(const std::string& section, const std::string& key,
                   float defaultValue = 0.0f) const {
        try { return std::stof(GetString(section, key)); }
        catch (...) { return defaultValue; }
    }

    bool GetBool(const std::string& section, const std::string& key,
                 bool defaultValue = false) const {
        auto value = GetString(section, key);
        std::transform(value.begin(), value.end(), value.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (value == "true" || value == "1" || value == "on" || value == "yes") return true;
        if (value == "false" || value == "0" || value == "off" || value == "no") return false;
        return defaultValue;
    }

    std::vector<std::string> GetStringList(const std::string& section,
                                           const std::string& key) const {
        const auto value = TryGet(section, key);
        return value.value_or(std::vector<std::string>{});
    }

    std::vector<int> GetIntList(const std::string& section, const std::string& key) const {
        std::vector<int> result;
        for (const auto& value : GetStringList(section, key)) {
            try { result.push_back(std::stoi(value)); }
            catch (...) { result.push_back(0); }
        }
        return result;
    }

    std::vector<float> GetFloatList(const std::string& section, const std::string& key) const {
        std::vector<float> result;
        for (const auto& value : GetStringList(section, key)) {
            try { result.push_back(std::stof(value)); }
            catch (...) { result.push_back(0.0f); }
        }
        return result;
    }

private:
    using Section = std::unordered_map<std::string, std::vector<std::string>>;
    std::unordered_map<std::string, Section> data_;

    static std::string Trim(const std::string& value) {
        std::size_t begin = 0;
        std::size_t end = value.size();
        while (begin < end && std::isspace(static_cast<unsigned char>(value[begin]))) ++begin;
        while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1]))) --end;
        return value.substr(begin, end - begin);
    }

    static std::vector<std::string> SplitCsv(const std::string& value) {
        std::vector<std::string> result;
        std::stringstream stream(value);
        std::string item;
        while (std::getline(stream, item, ',')) result.push_back(Trim(item));
        if (result.empty()) result.push_back({});
        return result;
    }
};

} // namespace mygame
