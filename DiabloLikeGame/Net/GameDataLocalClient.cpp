#include "GameDataLocalClient.h"
#include "../Core/IniParser.h"
#include <filesystem>

GameDataLocalClient::GameDataLocalClient(const std::string& configDir)
    : m_configDir(configDir)
{
}

bool GameDataLocalClient::Initialize()
{
    if (m_initialized) return true;
    
    bool success = true;
    success &= LoadPlayerConfig();
    success &= LoadEnemyTypes();
    
    m_initialized = success;
    return success;
}

bool GameDataLocalClient::LoadPlayerConfig()
{
    namespace fs = std::filesystem;
    
    const auto filepath = fs::path(m_configDir) / "player.ini";
    if (!fs::exists(filepath)) {
        return true;  // Use defaults if file doesn't exist
    }
    
    IniParser ini;
    if (!ini.Load(filepath.string())) {
        return true;  // Use defaults if can't load
    }
    
    m_playerData.maxHealth = ini.GetInt("Stats", "MaxHealth", 100);
    m_playerData.baseAttack = ini.GetFloat("Stats", "BaseAttack", 20.0f);
    m_playerData.attackVariation = ini.GetFloat("Stats", "AttackVariation", 0.1f);
    m_playerData.critChance = ini.GetFloat("Stats", "CritChance", 0.1f);
    m_playerData.critMultiplier = ini.GetFloat("Stats", "CritMultiplier", 2.0f);
    m_playerData.moveSpeed = ini.GetFloat("Movement", "MoveSpeed", 5.0f);
    m_playerData.punchDuration = ini.GetFloat("Combat", "PunchDuration", 0.25f);
    m_playerData.punchRange = ini.GetInt("Combat", "PunchRange", 1);
    
    return true;
}

bool GameDataLocalClient::LoadEnemyTypes()
{
    namespace fs = std::filesystem;
    
    const auto enemiesDir = fs::path(m_configDir) / "enemies";
    if (!fs::exists(enemiesDir) || !fs::is_directory(enemiesDir)) {
        return true;  // No enemies directory is OK
    }
    
    for (const auto& entry : fs::directory_iterator(enemiesDir)) {
        if (entry.path().extension() != ".ini") continue;
        
        IniParser ini;
        if (!ini.Load(entry.path().string())) continue;
        
        EnemyData data;
        data.id = ini.GetString("Identity", "Id", "unknown");
        data.displayName = ini.GetString("Identity", "DisplayName", "Unknown");
        
        data.maxHealth = ini.GetInt("Stats", "MaxHealth", 100);
        data.baseAttack = ini.GetFloat("Stats", "BaseAttack", 10.0f);
        data.attackVariation = ini.GetFloat("Stats", "AttackVariation", 0.1f);
        
        data.moveSpeed = ini.GetFloat("Movement", "MoveSpeed", 3.0f);
        data.wanderRadius = ini.GetInt("Movement", "WanderRadius", 5);
        data.pauseTimeMin = ini.GetFloat("Movement", "PauseTimeMin", 1.5f);
        data.pauseTimeMax = ini.GetFloat("Movement", "PauseTimeMax", 4.0f);
        
        data.aggression = ini.GetString("Behavior", "Aggression", "Defensive");
        data.attackCooldown = ini.GetFloat("Behavior", "AttackCooldown", 1.0f);
        data.attackRange = ini.GetInt("Behavior", "AttackRange", 1);
        
        data.colorR = ini.GetInt("Visual", "ColorR", 230);
        data.colorG = ini.GetInt("Visual", "ColorG", 41);
        data.colorB = ini.GetInt("Visual", "ColorB", 55);
        
        m_enemyTypes[data.id] = data;
        m_enemyTypeIds.push_back(data.id);
    }
    
    return true;
}

// ============== JSON Conversion ==============

Json::Value GameDataLocalClient::PlayerConfigToJson() const
{
    return Json::MakeObject()
        .Add("maxHealth", m_playerData.maxHealth)
        .Add("baseAttack", m_playerData.baseAttack)
        .Add("attackVariation", m_playerData.attackVariation)
        .Add("critChance", m_playerData.critChance)
        .Add("critMultiplier", m_playerData.critMultiplier)
        .Add("moveSpeed", m_playerData.moveSpeed)
        .Add("punchDuration", m_playerData.punchDuration)
        .Add("punchRange", m_playerData.punchRange)
        .Build();
}

Json::Value GameDataLocalClient::EnemyTypeToJson(const std::string& id) const
{
    auto it = m_enemyTypes.find(id);
    if (it == m_enemyTypes.end()) {
        return Json::MakeObject()
            .Add("error", "Enemy type not found")
            .Add("id", id)
            .Build();
    }
    
    const auto& e = it->second;
    return Json::MakeObject()
        .Add("id", e.id)
        .Add("displayName", e.displayName)
        .Add("maxHealth", e.maxHealth)
        .Add("baseAttack", e.baseAttack)
        .Add("attackVariation", e.attackVariation)
        .Add("moveSpeed", e.moveSpeed)
        .Add("wanderRadius", e.wanderRadius)
        .Add("pauseTimeMin", e.pauseTimeMin)
        .Add("pauseTimeMax", e.pauseTimeMax)
        .Add("aggression", e.aggression)
        .Add("attackCooldown", e.attackCooldown)
        .Add("attackRange", e.attackRange)
        .Add("colorR", e.colorR)
        .Add("colorG", e.colorG)
        .Add("colorB", e.colorB)
        .Build();
}

Json::Value GameDataLocalClient::AllEnemyTypesToJson() const
{
    Json::ArrayBuilder arr;
    for (const auto& id : m_enemyTypeIds) {
        arr.Add(EnemyTypeToJson(id));
    }
    
    return Json::MakeObject()
        .Add("enemyTypes", arr.Build())
        .Build();
}

// ============== Async Interface ==============

void GameDataLocalClient::RequestPlayerConfig(ResponseCallback callback)
{
    if (!m_initialized) Initialize();
    callback(true, PlayerConfigToJson().Stringify());
}

void GameDataLocalClient::RequestEnemyTypeList(ResponseCallback callback)
{
    if (!m_initialized) Initialize();
    
    Json::ArrayBuilder arr;
    for (const auto& id : m_enemyTypeIds) {
        arr.Add(id);
    }
    
    auto result = Json::MakeObject()
        .Add("enemyTypes", arr.Build())
        .Build();
    
    callback(true, result.Stringify());
}

void GameDataLocalClient::RequestEnemyType(const std::string& requestJson, ResponseCallback callback)
{
    if (!m_initialized) Initialize();
    
    try {
        auto request = Json::Value::Parse(requestJson);
        std::string id = request["id"].AsString();
        callback(true, EnemyTypeToJson(id).Stringify());
    } catch (...) {
        callback(false, R"({"error": "Invalid request JSON"})");
    }
}

void GameDataLocalClient::RequestAllEnemyTypes(ResponseCallback callback)
{
    if (!m_initialized) Initialize();
    callback(true, AllEnemyTypesToJson().Stringify());
}

// ============== Sync Interface ==============

std::string GameDataLocalClient::GetPlayerConfigSync()
{
    if (!m_initialized) Initialize();
    return PlayerConfigToJson().Stringify();
}

std::string GameDataLocalClient::GetEnemyTypeListSync()
{
    if (!m_initialized) Initialize();
    
    Json::ArrayBuilder arr;
    for (const auto& id : m_enemyTypeIds) {
        arr.Add(id);
    }
    
    return Json::MakeObject()
        .Add("enemyTypes", arr.Build())
        .Build()
        .Stringify();
}

std::string GameDataLocalClient::GetEnemyTypeSync(const std::string& id)
{
    if (!m_initialized) Initialize();
    return EnemyTypeToJson(id).Stringify();
}

std::string GameDataLocalClient::GetAllEnemyTypesSync()
{
    if (!m_initialized) Initialize();
    return AllEnemyTypesToJson().Stringify();
}
