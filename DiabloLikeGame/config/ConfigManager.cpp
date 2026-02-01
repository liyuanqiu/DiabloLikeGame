#include "ConfigManager.h"
#include "../Core/IniParser.h"
#include <filesystem>

ConfigManager& ConfigManager::Instance()
{
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::LoadAll(std::string_view configDir)
{
    namespace fs = std::filesystem;
    
    const fs::path configPath(configDir);
    
    // Load player config
    const auto playerConfigPath = configPath / "player.ini";
    if (fs::exists(playerConfigPath)) {
        LoadPlayerConfig(playerConfigPath.string());
    }
    
    // Load enemy types from enemies/ subdirectory
    const auto enemiesPath = configPath / "enemies";
    if (fs::exists(enemiesPath) && fs::is_directory(enemiesPath)) {
        for (const auto& entry : fs::directory_iterator(enemiesPath)) {
            if (entry.path().extension() == ".ini") {
                LoadEnemyType(entry.path().string());
            }
        }
    }
    
    return true;
}

bool ConfigManager::LoadPlayerConfig(std::string_view filepath)
{
    IniParser ini;
    if (!ini.Load(filepath)) {
        return false;
    }
    
    // Stats section
    m_playerConfig.maxHealth = ini.GetInt("Stats", "MaxHealth", 100);
    m_playerConfig.baseAttack = ini.GetFloat("Stats", "BaseAttack", 20.0f);
    m_playerConfig.attackVariation = ini.GetFloat("Stats", "AttackVariation", 0.1f);
    m_playerConfig.critChance = ini.GetFloat("Stats", "CritChance", 0.1f);
    m_playerConfig.critMultiplier = ini.GetFloat("Stats", "CritMultiplier", 2.0f);
    
    // Movement section
    m_playerConfig.moveSpeed = ini.GetFloat("Movement", "MoveSpeed", 5.0f);
    
    // Combat section
    m_playerConfig.punchDuration = ini.GetFloat("Combat", "PunchDuration", 0.25f);
    m_playerConfig.punchRange = ini.GetInt("Combat", "PunchRange", 1);
    
    return true;
}

bool ConfigManager::LoadEnemyType(std::string_view filepath)
{
    IniParser ini;
    if (!ini.Load(filepath)) {
        return false;
    }
    
    EnemyTypeConfig config;
    
    // Identity section
    config.id = ini.GetString("Identity", "Id", "unknown");
    config.displayName = ini.GetString("Identity", "DisplayName", "Unknown Enemy");
    
    // Stats section
    config.maxHealth = ini.GetInt("Stats", "MaxHealth", 100);
    config.baseAttack = ini.GetFloat("Stats", "BaseAttack", 10.0f);
    config.attackVariation = ini.GetFloat("Stats", "AttackVariation", 0.1f);
    
    // Movement section
    config.moveSpeed = ini.GetFloat("Movement", "MoveSpeed", 3.0f);
    config.wanderRadius = ini.GetInt("Movement", "WanderRadius", 5);
    config.pauseTimeMin = ini.GetFloat("Movement", "PauseTimeMin", 1.5f);
    config.pauseTimeMax = ini.GetFloat("Movement", "PauseTimeMax", 4.0f);
    
    // Behavior section
    const auto aggressionStr = ini.GetString("Behavior", "Aggression", "Defensive");
    config.aggression = ParseAggressionType(aggressionStr);
    config.attackCooldown = ini.GetFloat("Behavior", "AttackCooldown", 1.0f);
    config.attackRange = ini.GetInt("Behavior", "AttackRange", 1);
    
    // Visual section
    config.colorR = static_cast<uint8_t>(ini.GetInt("Visual", "ColorR", 230));
    config.colorG = static_cast<uint8_t>(ini.GetInt("Visual", "ColorG", 41));
    config.colorB = static_cast<uint8_t>(ini.GetInt("Visual", "ColorB", 55));
    
    // Store the config
    m_enemyTypes[config.id] = config;
    m_enemyTypeIds.push_back(config.id);
    
    return true;
}

const EnemyTypeConfig* ConfigManager::GetEnemyType(std::string_view id) const
{
    const auto it = m_enemyTypes.find(std::string(id));
    if (it != m_enemyTypes.end()) {
        return &it->second;
    }
    return nullptr;
}

const EnemyTypeConfig& ConfigManager::GetDefaultEnemyType() const
{
    if (!m_enemyTypeIds.empty()) {
        const auto it = m_enemyTypes.find(m_enemyTypeIds[0]);
        if (it != m_enemyTypes.end()) {
            return it->second;
        }
    }
    return m_defaultEnemyType;
}
