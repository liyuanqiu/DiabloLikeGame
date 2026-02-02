#pragma once

#include "Common/MapGenerator.h"
#include <string>
#include <string_view>

// Loads map generator configuration from INI file
class MapGeneratorConfig {
public:
    // Load configuration from INI file
    static bool Load(const char* filename = "config/mapgen.ini");
    
    // Get configuration by preset name ("Default", "Small", "Large")
    [[nodiscard]] static MapGenerator::Config GetPreset(std::string_view presetName = "Default");
    
    // Check if a preset exists
    [[nodiscard]] static bool HasPreset(std::string_view presetName);
    
    // Get default configuration (compile-time fallback)
    [[nodiscard]] static MapGenerator::Config GetDefault() { return s_defaultConfig; }
    [[nodiscard]] static MapGenerator::Config GetSmall() { return s_smallConfig; }
    [[nodiscard]] static MapGenerator::Config GetLarge() { return s_largeConfig; }

private:
    inline static MapGenerator::Config s_defaultConfig{};
    inline static MapGenerator::Config s_smallConfig{};
    inline static MapGenerator::Config s_largeConfig{};
    inline static bool s_loaded = false;
};
