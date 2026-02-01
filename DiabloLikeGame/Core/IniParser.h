#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <optional>
#include "raylib.h"

// Simple INI file parser
class IniParser {
public:
    // Load INI file
    [[nodiscard]] bool Load(std::string_view filename);
    
    // Get value from section
    [[nodiscard]] std::optional<std::string> GetString(
        std::string_view section, std::string_view key) const;
    
    [[nodiscard]] std::optional<int> GetInt(
        std::string_view section, std::string_view key) const;
    
    [[nodiscard]] std::optional<float> GetFloat(
        std::string_view section, std::string_view key) const;
    
    [[nodiscard]] std::optional<bool> GetBool(
        std::string_view section, std::string_view key) const;
    
    [[nodiscard]] std::optional<Color> GetColor(
        std::string_view section, std::string_view key) const;
    
    // Get with default value
    [[nodiscard]] std::string GetString(
        std::string_view section, std::string_view key, std::string_view defaultValue) const;
    
    [[nodiscard]] int GetInt(
        std::string_view section, std::string_view key, int defaultValue) const;
    
    [[nodiscard]] float GetFloat(
        std::string_view section, std::string_view key, float defaultValue) const;
    
    [[nodiscard]] bool GetBool(
        std::string_view section, std::string_view key, bool defaultValue) const;
    
    [[nodiscard]] Color GetColor(
        std::string_view section, std::string_view key, Color defaultValue) const;

private:
    // section -> (key -> value)
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_data;
};
