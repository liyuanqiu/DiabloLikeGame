#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include "IniParser.h"

namespace {
    std::string Trim(std::string_view str) {
        const auto start = str.find_first_not_of(" \t\r\n");
        if (start == std::string_view::npos) return "";
        const auto end = str.find_last_not_of(" \t\r\n");
        return std::string(str.substr(start, end - start + 1));
    }
}

bool IniParser::Load(std::string_view filename) {
std::ifstream file{std::string{filename}};
if (!file.is_open()) return false;
    
    std::string currentSection;
    std::string line;
    
    while (std::getline(file, line)) {
        line = Trim(line);
        if (line.empty() || line[0] == ';' || line[0] == '#') continue;
        
        if (line[0] == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.size() - 2);
            continue;
        }
        
        const auto eqPos = line.find('=');
        if (eqPos != std::string::npos) {
            auto key = Trim(line.substr(0, eqPos));
            auto value = Trim(line.substr(eqPos + 1));
            m_data[currentSection][key] = value;
        }
    }
    return true;
}

std::optional<std::string> IniParser::GetString(
    std::string_view section, std::string_view key) const {
    
    if (auto secIt = m_data.find(std::string(section)); secIt != m_data.end()) {
        if (auto keyIt = secIt->second.find(std::string(key)); keyIt != secIt->second.end()) {
            return keyIt->second;
        }
    }
    return std::nullopt;
}

std::optional<int> IniParser::GetInt(
    std::string_view section, std::string_view key) const {
    
    if (auto str = GetString(section, key)) {
        try { return std::stoi(*str); }
        catch (...) { return std::nullopt; }
    }
    return std::nullopt;
}

std::optional<float> IniParser::GetFloat(
    std::string_view section, std::string_view key) const {
    
    if (auto str = GetString(section, key)) {
        try { return std::stof(*str); }
        catch (...) { return std::nullopt; }
    }
    return std::nullopt;
}

std::optional<bool> IniParser::GetBool(
    std::string_view section, std::string_view key) const {
    
    if (auto str = GetString(section, key)) {
        auto lower = *str;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (lower == "true" || lower == "1" || lower == "yes") return true;
        if (lower == "false" || lower == "0" || lower == "no") return false;
    }
    return std::nullopt;
}

std::optional<Color> IniParser::GetColor(
    std::string_view section, std::string_view key) const {
    
    if (auto str = GetString(section, key)) {
        std::istringstream ss(*str);
        int r, g, b, a = 255;
        char comma;
        if (ss >> r >> comma >> g >> comma >> b) {
            ss >> comma >> a;  // Alpha is optional
            return Color{
                static_cast<unsigned char>(r),
                static_cast<unsigned char>(g),
                static_cast<unsigned char>(b),
                static_cast<unsigned char>(a)
            };
        }
    }
    return std::nullopt;
}

std::string IniParser::GetString(
    std::string_view section, std::string_view key, std::string_view defaultValue) const {
    return GetString(section, key).value_or(std::string(defaultValue));
}

int IniParser::GetInt(
    std::string_view section, std::string_view key, int defaultValue) const {
    return GetInt(section, key).value_or(defaultValue);
}

float IniParser::GetFloat(
    std::string_view section, std::string_view key, float defaultValue) const {
    return GetFloat(section, key).value_or(defaultValue);
}

bool IniParser::GetBool(
    std::string_view section, std::string_view key, bool defaultValue) const {
    return GetBool(section, key).value_or(defaultValue);
}

Color IniParser::GetColor(
    std::string_view section, std::string_view key, Color defaultValue) const {
    return GetColor(section, key).value_or(defaultValue);
}
