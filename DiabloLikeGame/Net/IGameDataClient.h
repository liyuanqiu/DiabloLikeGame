#pragma once

#include <string>
#include <functional>

// Interface for game data client
// All communication uses JSON strings for abstraction
// Future network implementation will handle JSON <-> server protocol conversion
class IGameDataClient {
public:
    virtual ~IGameDataClient() = default;
    
    // Response callback type
    using ResponseCallback = std::function<void(bool success, const std::string& jsonResponse)>;
    
    // ============== Player Data ==============
    
    // Get player configuration
    // Response JSON: { "maxHealth": int, "baseAttack": float, "critChance": float, ... }
    virtual void RequestPlayerConfig(ResponseCallback callback) = 0;
    
    // ============== Enemy Data ==============
    
    // Get list of all enemy type IDs
    // Response JSON: { "enemyTypes": ["goblin", "skeleton", ...] }
    virtual void RequestEnemyTypeList(ResponseCallback callback) = 0;
    
    // Get specific enemy type configuration
    // Request JSON: { "id": "goblin" }
    // Response JSON: { "id": "goblin", "displayName": "Goblin", "maxHealth": int, ... }
    virtual void RequestEnemyType(const std::string& requestJson, ResponseCallback callback) = 0;
    
    // Get all enemy types at once
    // Response JSON: { "enemyTypes": [ {...}, {...}, ... ] }
    virtual void RequestAllEnemyTypes(ResponseCallback callback) = 0;
    
    // ============== Synchronous convenience methods ==============
    // These block until data is ready - useful for initialization
    
    [[nodiscard]] virtual std::string GetPlayerConfigSync() = 0;
    [[nodiscard]] virtual std::string GetEnemyTypeListSync() = 0;
    [[nodiscard]] virtual std::string GetEnemyTypeSync(const std::string& id) = 0;
    [[nodiscard]] virtual std::string GetAllEnemyTypesSync() = 0;
};
