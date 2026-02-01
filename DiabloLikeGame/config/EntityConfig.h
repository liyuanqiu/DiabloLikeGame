#pragma once

#include <string>
#include <cstdint>

// Enemy aggression behavior types
enum class AggressionType : uint8_t {
    Passive,     // Never attacks
    Defensive,   // Only attacks when hit first
    Aggressive   // Attacks on sight
};

// Player configuration data
struct PlayerConfig {
    // Stats
    int maxHealth = 100;
    float baseAttack = 20.0f;
    float attackVariation = 0.1f;    // ¡À10%
    float critChance = 0.1f;         // 10%
    float critMultiplier = 2.0f;     // 2x damage
    
    // Movement
    float moveSpeed = 5.0f;
    
    // Combat
    float punchDuration = 0.25f;     // Animation time
    int punchRange = 1;              // Tiles
};

// Enemy type configuration data
struct EnemyTypeConfig {
    // Identity
    std::string id;                  // Unique identifier (e.g., "goblin", "skeleton")
    std::string displayName;         // Display name
    
    // Stats
    int maxHealth = 100;
    float baseAttack = 10.0f;
    float attackVariation = 0.1f;    // ¡À10%
    
    // Movement
    float moveSpeed = 3.0f;
    int wanderRadius = 5;
    float pauseTimeMin = 1.5f;
    float pauseTimeMax = 4.0f;
    
    // Combat behavior
    AggressionType aggression = AggressionType::Defensive;
    float attackCooldown = 1.0f;     // Seconds between attacks
    int attackRange = 1;             // Tiles
    
    // Visual (for future use)
    uint8_t colorR = 230;
    uint8_t colorG = 41;
    uint8_t colorB = 55;
};

// Convert string to AggressionType
inline AggressionType ParseAggressionType(const std::string& str)
{
    if (str == "passive" || str == "Passive") return AggressionType::Passive;
    if (str == "aggressive" || str == "Aggressive") return AggressionType::Aggressive;
    return AggressionType::Defensive;  // Default
}

// Convert AggressionType to string
inline const char* AggressionTypeToString(AggressionType type)
{
    switch (type) {
        case AggressionType::Passive: return "Passive";
        case AggressionType::Aggressive: return "Aggressive";
        case AggressionType::Defensive: return "Defensive";
    }
    return "Defensive";
}
