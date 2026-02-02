#include "MapConfig.h"
#include "../Core/IniParser.h"
#include <sstream>
#include <algorithm>

GameplayDefaults& GameplayDefaults::Instance()
{
    static GameplayDefaults instance;
    return instance;
}

bool GameplayDefaults::Load(const char* filename)
{
    IniParser ini;
    if (!ini.Load(filename)) {
        return false;
    }
    
    // Spawning
    m_defaults.enemySpawnRate = ini.GetFloat("Spawning", "EnemySpawnRate", 0.10f);
    m_defaults.safeRadiusFromPlayer = ini.GetInt("Spawning", "SafeRadiusFromPlayer", 5);
    
    // Parse allowed enemy types
    const std::string types = ini.GetString("Spawning", "AllowedEnemyTypes", "");
    if (!types.empty()) {
        std::istringstream iss(types);
        std::string type;
        while (std::getline(iss, type, ',')) {
            // Trim whitespace
            type.erase(0, type.find_first_not_of(" \t"));
            type.erase(type.find_last_not_of(" \t") + 1);
            if (!type.empty()) {
                m_defaults.allowedEnemyTypes.push_back(type);
            }
        }
    }
    
    // Combat
    m_defaults.damageMultiplier = ini.GetFloat("Combat", "DamageMultiplier", 1.0f);
    m_defaults.enemyCombatTimeout = ini.GetFloat("Combat", "EnemyCombatTimeout", 20.0f);
    m_defaults.enemyLeashDistance = ini.GetInt("Combat", "EnemyLeashDistance", 30);
    m_defaults.enemyVisionRange = ini.GetInt("Combat", "EnemyVisionRange", 8);
    
    // Difficulty
    m_defaults.difficultyMultiplier = ini.GetFloat("Difficulty", "Multiplier", 1.0f);
    
    return true;
}

MapConfig MapConfigLoader::Load(const std::string& mapPath)
{
    // Get defaults first
    MapConfig config = GameplayDefaults::Instance().GetDefaults();
    
    // Derive INI path from map path (maps/default.map -> maps/default.ini)
    std::string iniPath = mapPath;
    const auto dotPos = iniPath.rfind('.');
    if (dotPos != std::string::npos) {
        iniPath = iniPath.substr(0, dotPos) + ".ini";
    } else {
        iniPath += ".ini";
    }
    
    return LoadFromFile(iniPath, config);
}

MapConfig MapConfigLoader::LoadFromFile(const std::string& iniPath, 
                                         const MapConfig& defaults)
{
    MapConfig config = defaults;
    
    IniParser ini;
    if (!ini.Load(iniPath.c_str())) {
        // No map-specific config, use defaults
        return config;
    }
    
    // Info
    config.displayName = ini.GetString("Info", "DisplayName", config.displayName);
    config.description = ini.GetString("Info", "Description", config.description);
    
    // Spawning (override if present)
    if (ini.HasKey("Spawning", "EnemySpawnRate")) {
        config.enemySpawnRate = ini.GetFloat("Spawning", "EnemySpawnRate", config.enemySpawnRate);
    }
    if (ini.HasKey("Spawning", "SafeRadiusFromPlayer")) {
        config.safeRadiusFromPlayer = ini.GetInt("Spawning", "SafeRadiusFromPlayer", config.safeRadiusFromPlayer);
    }
    
    // Parse allowed enemy types (completely replaces default if specified)
    const std::string types = ini.GetString("Spawning", "AllowedEnemyTypes", "");
    if (!types.empty()) {
        config.allowedEnemyTypes.clear();
        std::istringstream iss(types);
        std::string type;
        while (std::getline(iss, type, ',')) {
            type.erase(0, type.find_first_not_of(" \t"));
            type.erase(type.find_last_not_of(" \t") + 1);
            if (!type.empty()) {
                config.allowedEnemyTypes.push_back(type);
            }
        }
    }
    
    // Combat
    if (ini.HasKey("Combat", "DamageMultiplier")) {
        config.damageMultiplier = ini.GetFloat("Combat", "DamageMultiplier", config.damageMultiplier);
    }
    if (ini.HasKey("Combat", "EnemyCombatTimeout")) {
        config.enemyCombatTimeout = ini.GetFloat("Combat", "EnemyCombatTimeout", config.enemyCombatTimeout);
    }
    if (ini.HasKey("Combat", "EnemyLeashDistance")) {
        config.enemyLeashDistance = ini.GetInt("Combat", "EnemyLeashDistance", config.enemyLeashDistance);
    }
    if (ini.HasKey("Combat", "EnemyVisionRange")) {
        config.enemyVisionRange = ini.GetInt("Combat", "EnemyVisionRange", config.enemyVisionRange);
    }
    
    // Difficulty
    if (ini.HasKey("Difficulty", "Multiplier")) {
        config.difficultyMultiplier = ini.GetFloat("Difficulty", "Multiplier", config.difficultyMultiplier);
    }
    
    return config;
}
