#pragma once

#include "IGameDataClient.h"
#include "Json.h"
#include <string>
#include <unordered_map>

// Local implementation of IGameDataClient
// Reads from local config files instead of server
// Used for offline play and development
class GameDataLocalClient : public IGameDataClient {
public:
    explicit GameDataLocalClient(const std::string& configDir = "Config");
    ~GameDataLocalClient() override = default;
    
    // Load all data from config directory
    bool Initialize();
    
    // ============== Async Interface (callbacks execute immediately for local) ==============
    
    void RequestPlayerConfig(ResponseCallback callback) override;
    void RequestEnemyTypeList(ResponseCallback callback) override;
    void RequestEnemyType(const std::string& requestJson, ResponseCallback callback) override;
    void RequestAllEnemyTypes(ResponseCallback callback) override;
    
    // ============== Sync Interface ==============
    
    [[nodiscard]] std::string GetPlayerConfigSync() override;
    [[nodiscard]] std::string GetEnemyTypeListSync() override;
    [[nodiscard]] std::string GetEnemyTypeSync(const std::string& id) override;
    [[nodiscard]] std::string GetAllEnemyTypesSync() override;

private:
    // Load player config from INI
    bool LoadPlayerConfig();
    
    // Load enemy types from INI files in enemies/ subdirectory
    bool LoadEnemyTypes();
    
    // Convert loaded data to JSON
    [[nodiscard]] Json::Value PlayerConfigToJson() const;
    [[nodiscard]] Json::Value EnemyTypeToJson(const std::string& id) const;
    [[nodiscard]] Json::Value AllEnemyTypesToJson() const;
    
    std::string m_configDir;
    
    // Cached player config
    struct PlayerData {
        int maxHealth = 100;
        float baseAttack = 20.0f;
        float attackVariation = 0.1f;
        float critChance = 0.1f;
        float critMultiplier = 2.0f;
        float moveSpeed = 5.0f;
        float punchDuration = 0.25f;
        int punchRange = 1;
    };
    PlayerData m_playerData;
    
    // Cached enemy types
    struct EnemyData {
        std::string id;
        std::string displayName;
        int maxHealth = 100;
        float baseAttack = 10.0f;
        float attackVariation = 0.1f;
        float moveSpeed = 3.0f;
        int wanderRadius = 5;
        float pauseTimeMin = 1.5f;
        float pauseTimeMax = 4.0f;
        std::string aggression = "Defensive";
        float attackCooldown = 1.0f;
        int attackRange = 1;
        int colorR = 230;
        int colorG = 41;
        int colorB = 55;
    };
    std::unordered_map<std::string, EnemyData> m_enemyTypes;
    std::vector<std::string> m_enemyTypeIds;
    
    bool m_initialized = false;
};
