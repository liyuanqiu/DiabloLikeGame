#pragma once

#include <string>
#include <vector>

// Map-specific gameplay configuration
// Loaded from maps/xxx.ini, falls back to gameplay/defaults.ini
struct MapConfig {
    // Info
    std::string displayName;
    std::string description;
    
    // Spawning
    float enemySpawnRate = 0.10f;
    int safeRadiusFromPlayer = 5;
    std::vector<std::string> allowedEnemyTypes;  // Empty = all types
    
    // Combat
    float damageMultiplier = 1.0f;
    float enemyCombatTimeout = 20.0f;
    int enemyLeashDistance = 30;
    int enemyVisionRange = 8;
    
    // Difficulty
    float difficultyMultiplier = 1.0f;
    
    // Computed values (after applying difficulty)
    [[nodiscard]] float GetEffectiveSpawnRate() const {
        return enemySpawnRate * difficultyMultiplier;
    }
    
    [[nodiscard]] float GetEffectiveDamageMultiplier() const {
        return damageMultiplier * difficultyMultiplier;
    }
};

// Global gameplay defaults (loaded from gameplay/defaults.ini)
class GameplayDefaults {
public:
    static GameplayDefaults& Instance();
    
    // Load global defaults
    bool Load(const char* filename = "config/gameplay/defaults.ini");
    
    // Get a copy of default config (for map to override)
    [[nodiscard]] MapConfig GetDefaults() const { return m_defaults; }

private:
    GameplayDefaults() = default;
    MapConfig m_defaults;
};

// Map configuration loader
class MapConfigLoader {
public:
    // Load map config, falling back to global defaults
    // Looks for mapPath.ini (e.g., "maps/default.map" -> "maps/default.ini")
    [[nodiscard]] static MapConfig Load(const std::string& mapPath);
    
    // Load map config from specific INI file
    [[nodiscard]] static MapConfig LoadFromFile(const std::string& iniPath, 
                                                  const MapConfig& defaults);
};
