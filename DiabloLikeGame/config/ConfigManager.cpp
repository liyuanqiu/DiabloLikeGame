#include "ConfigManager.h"
#include "../Core/IniParser.h"
#include "../Net/IGameDataClient.h"
#include "../Net/GameDataClientFactory.h"
#include "../Net/Json.h"

ConfigManager& ConfigManager::Instance()
{
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::Initialize(IGameDataClient* client)
{
    if (!client) {
        client = &GameDataClientFactory::GetInstance();
    }
    
    // Get player config
    std::string playerJson = client->GetPlayerConfigSync();
    if (!ParsePlayerConfig(playerJson)) {
        return false;
    }
    
    // Get all enemy types
    std::string enemiesJson = client->GetAllEnemyTypesSync();
    if (!ParseEnemyTypes(enemiesJson)) {
        return false;
    }
    
    return true;
}

bool ConfigManager::LoadAll(std::string_view configDir)
{
    // Load game.ini settings first
    LoadGameSettings(configDir);
    
    // Set up local client with specified directory
    GameDataClientFactory::SetClientType(
        GameDataClientFactory::ClientType::Local, 
        std::string(configDir)
    );
    return Initialize();
}

void ConfigManager::LoadGameSettings(std::string_view configDir)
{
    const std::string iniPath = std::string(configDir) + "/game.ini";
    IniParser ini;
    if (ini.Load(iniPath.c_str())) {
        m_enemySpawnRate = ini.GetFloat("Enemy", "SpawnRate", m_enemySpawnRate);
        m_defaultMapPath = ini.GetString("Paths", "DefaultMap", m_defaultMapPath);
    }
}

bool ConfigManager::ParsePlayerConfig(const std::string& json)
{
    try {
        auto data = Json::Value::Parse(json);
        
        if (auto val = data["maxHealth"].GetInt()) m_playerConfig.maxHealth = static_cast<int>(*val);
        if (auto val = data["baseAttack"].GetFloat()) m_playerConfig.baseAttack = static_cast<float>(*val);
        if (auto val = data["attackVariation"].GetFloat()) m_playerConfig.attackVariation = static_cast<float>(*val);
        if (auto val = data["critChance"].GetFloat()) m_playerConfig.critChance = static_cast<float>(*val);
        if (auto val = data["critMultiplier"].GetFloat()) m_playerConfig.critMultiplier = static_cast<float>(*val);
        if (auto val = data["moveSpeed"].GetFloat()) m_playerConfig.moveSpeed = static_cast<float>(*val);
        if (auto val = data["punchDuration"].GetFloat()) m_playerConfig.punchDuration = static_cast<float>(*val);
        if (auto val = data["punchRange"].GetInt()) m_playerConfig.punchRange = static_cast<int>(*val);
        
        return true;
    } catch (...) {
        return false;
    }
}

bool ConfigManager::ParseEnemyTypes(const std::string& json)
{
    try {
        auto data = Json::Value::Parse(json);
        
        if (!data.Has("enemyTypes") || !data["enemyTypes"].IsArray()) {
            return false;
        }
        
        m_enemyTypes.clear();
        m_enemyTypeIds.clear();
        
        for (const auto& enemy : data["enemyTypes"].AsArray()) {
            EnemyTypeConfig config;
            
            if (auto val = enemy["id"].GetString()) config.id = *val;
            if (auto val = enemy["displayName"].GetString()) config.displayName = *val;
            if (auto val = enemy["maxHealth"].GetInt()) config.maxHealth = static_cast<int>(*val);
            if (auto val = enemy["baseAttack"].GetFloat()) config.baseAttack = static_cast<float>(*val);
            if (auto val = enemy["attackVariation"].GetFloat()) config.attackVariation = static_cast<float>(*val);
            if (auto val = enemy["moveSpeed"].GetFloat()) config.moveSpeed = static_cast<float>(*val);
            if (auto val = enemy["wanderRadius"].GetInt()) config.wanderRadius = static_cast<int>(*val);
            if (auto val = enemy["pauseTimeMin"].GetFloat()) config.pauseTimeMin = static_cast<float>(*val);
            if (auto val = enemy["pauseTimeMax"].GetFloat()) config.pauseTimeMax = static_cast<float>(*val);
            if (auto val = enemy["aggression"].GetString()) config.aggression = ParseAggressionType(*val);
            if (auto val = enemy["attackCooldown"].GetFloat()) config.attackCooldown = static_cast<float>(*val);
            if (auto val = enemy["attackRange"].GetInt()) config.attackRange = static_cast<int>(*val);
            if (auto val = enemy["colorR"].GetInt()) config.colorR = static_cast<uint8_t>(*val);
            if (auto val = enemy["colorG"].GetInt()) config.colorG = static_cast<uint8_t>(*val);
            if (auto val = enemy["colorB"].GetInt()) config.colorB = static_cast<uint8_t>(*val);
            
            m_enemyTypes[config.id] = config;
            m_enemyTypeIds.push_back(config.id);
        }
        
        return true;
    } catch (...) {
        return false;
    }
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
    static EnemyTypeConfig defaultConfig;
    
    if (!m_enemyTypeIds.empty()) {
        const auto it = m_enemyTypes.find(m_enemyTypeIds[0]);
        if (it != m_enemyTypes.end()) {
            return it->second;
        }
    }
    return defaultConfig;
}
