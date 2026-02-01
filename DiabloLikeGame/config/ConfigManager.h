#pragma once

#include "EntityConfig.h"
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <optional>

// Loads and manages game configuration
class ConfigManager {
public:
    static ConfigManager& Instance();
    
    // Load all configuration files
    bool LoadAll(std::string_view configDir = "config");
    
    // Load specific configs
    bool LoadPlayerConfig(std::string_view filepath);
    bool LoadEnemyType(std::string_view filepath);
    
    // Get configurations
    [[nodiscard]] const PlayerConfig& GetPlayerConfig() const { return m_playerConfig; }
    [[nodiscard]] const EnemyTypeConfig* GetEnemyType(std::string_view id) const;
    [[nodiscard]] const std::vector<std::string>& GetEnemyTypeIds() const { return m_enemyTypeIds; }
    
    // Get default enemy type (first loaded, or fallback)
    [[nodiscard]] const EnemyTypeConfig& GetDefaultEnemyType() const;

private:
    ConfigManager() = default;
    
    PlayerConfig m_playerConfig;
    std::unordered_map<std::string, EnemyTypeConfig> m_enemyTypes;
    std::vector<std::string> m_enemyTypeIds;
    EnemyTypeConfig m_defaultEnemyType;  // Fallback
};
