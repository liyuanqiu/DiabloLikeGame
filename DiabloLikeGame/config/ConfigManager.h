#pragma once

#include "EntityConfig.h"
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

// Forward declaration
class IGameDataClient;

// Loads and manages game configuration via IGameDataClient
class ConfigManager {
public:
    static ConfigManager& Instance();
    
    // Initialize using game data client (loads all config)
    // If client is null, uses GameDataClientFactory::GetInstance()
    bool Initialize(IGameDataClient* client = nullptr);
    
    // Load all configuration from directory (includes game.ini)
    bool LoadAll(std::string_view configDir = "Config");
    
    // Get configurations
    [[nodiscard]] const PlayerConfig& GetPlayerConfig() const { return m_playerConfig; }
    [[nodiscard]] const EnemyTypeConfig* GetEnemyType(std::string_view id) const;
    [[nodiscard]] const std::vector<std::string>& GetEnemyTypeIds() const { return m_enemyTypeIds; }
    
    // Get default enemy type (first loaded, or fallback)
    [[nodiscard]] const EnemyTypeConfig& GetDefaultEnemyType() const;
    
    // Game config accessors (from game.ini)
    [[nodiscard]] float EnemySpawnRate() const { return m_enemySpawnRate; }
    [[nodiscard]] const std::string& DefaultMapPath() const { return m_defaultMapPath; }

private:
    ConfigManager() = default;
    
    // Parse JSON responses into config structures
    bool ParsePlayerConfig(const std::string& json);
    bool ParseEnemyTypes(const std::string& json);
    
    // Load game.ini settings
    void LoadGameSettings(std::string_view configDir);
    
    PlayerConfig m_playerConfig;
    std::unordered_map<std::string, EnemyTypeConfig> m_enemyTypes;
    std::vector<std::string> m_enemyTypeIds;
    
    // Game settings from game.ini
    float m_enemySpawnRate = 0.10f;
    std::string m_defaultMapPath = "maps/default.map";
};
