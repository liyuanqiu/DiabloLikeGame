#include "MapGeneratorConfig.h"
#include "../Core/IniParser.h"

bool MapGeneratorConfig::Load(const char* filename)
{
    IniParser ini;
    if (!ini.Load(filename)) {
        return false;
    }
    
    // Load Default preset
    s_defaultConfig.width = ini.GetInt("Default", "Width", 200);
    s_defaultConfig.height = ini.GetInt("Default", "Height", 200);
    s_defaultConfig.wallDensity = ini.GetFloat("Default", "WallDensity", 0.45f);
    s_defaultConfig.smoothIterations = ini.GetInt("Default", "SmoothIterations", 5);
    s_defaultConfig.wallThreshold = ini.GetInt("Default", "WallThreshold", 4);
    s_defaultConfig.waterChance = ini.GetFloat("Default", "WaterChance", 0.02f);
    s_defaultConfig.seed = static_cast<unsigned int>(ini.GetInt("Default", "Seed", 0));
    
    // Load Small preset
    s_smallConfig.width = ini.GetInt("Small", "Width", 50);
    s_smallConfig.height = ini.GetInt("Small", "Height", 50);
    s_smallConfig.wallDensity = ini.GetFloat("Small", "WallDensity", 0.40f);
    s_smallConfig.smoothIterations = ini.GetInt("Small", "SmoothIterations", 4);
    s_smallConfig.wallThreshold = ini.GetInt("Small", "WallThreshold", 4);
    s_smallConfig.waterChance = ini.GetFloat("Small", "WaterChance", 0.01f);
    s_smallConfig.seed = static_cast<unsigned int>(ini.GetInt("Small", "Seed", 0));
    
    // Load Large preset
    s_largeConfig.width = ini.GetInt("Large", "Width", 400);
    s_largeConfig.height = ini.GetInt("Large", "Height", 400);
    s_largeConfig.wallDensity = ini.GetFloat("Large", "WallDensity", 0.45f);
    s_largeConfig.smoothIterations = ini.GetInt("Large", "SmoothIterations", 6);
    s_largeConfig.wallThreshold = ini.GetInt("Large", "WallThreshold", 4);
    s_largeConfig.waterChance = ini.GetFloat("Large", "WaterChance", 0.03f);
    s_largeConfig.seed = static_cast<unsigned int>(ini.GetInt("Large", "Seed", 0));
    
    s_loaded = true;
    return true;
}

MapGenerator::Config MapGeneratorConfig::GetPreset(std::string_view presetName)
{
    if (presetName == "Small") return s_smallConfig;
    if (presetName == "Large") return s_largeConfig;
    return s_defaultConfig;  // Default fallback
}

bool MapGeneratorConfig::HasPreset(std::string_view presetName)
{
    return presetName == "Default" || presetName == "Small" || presetName == "Large";
}
