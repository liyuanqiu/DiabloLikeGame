#pragma once

#include <vector>
#include <set>
#include <cstdint>

// Forward declarations
class Entity;
class Enemy;
class Player;

// Combat behavior state for enemies
enum class CombatBehavior : uint8_t {
    Wandering,      // Normal wandering behavior
    Fleeing,        // Running away (for Passive enemies)
    Chasing,        // Pursuing a target
    Attacking,      // In attack range, attacking
    Returning       // Returning to spawn point
};

// Combat state for enemies
struct EnemyCombatState {
    bool inCombat = false;
    CombatBehavior behavior = CombatBehavior::Wandering;
    
    // Timing
    float combatTimer = 0.0f;           // Time since last successful action
    float lastDamageReceivedTime = 0.0f; // Time since last took damage
    
    // Threat tracking
    std::set<Entity*> threatList;        // All entities that have damaged us
    Entity* currentTarget = nullptr;     // Current chase/attack target
    
    // Spawn point for returning
    int spawnX = 0;
    int spawnY = 0;
    
    // Configuration
    static constexpr float kCombatTimeout = 20.0f;    // Seconds before giving up
    static constexpr int kLeashDistance = 30;         // Max distance from spawn
    static constexpr int kVisionRange = 8;            // Tiles for aggro detection
    
    // Reset combat state
    void Reset() noexcept {
        inCombat = false;
        behavior = CombatBehavior::Wandering;
        combatTimer = 0.0f;
        lastDamageReceivedTime = 0.0f;
        threatList.clear();
        currentTarget = nullptr;
    }
    
    // Enter combat with a target
    void EnterCombat(Entity* attacker) {
        inCombat = true;
        combatTimer = 0.0f;
        if (attacker) {
            threatList.insert(attacker);
            currentTarget = attacker;
        }
    }
    
    // Record damage received
    void OnDamageReceived(Entity* attacker) {
        lastDamageReceivedTime = 0.0f;
        if (attacker) {
            threatList.insert(attacker);
            currentTarget = attacker;
        }
    }
    
    // Record successful attack
    void OnAttackSuccess() noexcept {
        combatTimer = 0.0f;  // Reset timeout
    }
    
    // Check if should give up combat
    [[nodiscard]] bool ShouldGiveUp() const noexcept {
        return combatTimer >= kCombatTimeout;
    }
    
    // Check if target is in vision range
    [[nodiscard]] static bool IsInVisionRange(int x1, int y1, int x2, int y2) noexcept {
        const int dx = x2 - x1;
        const int dy = y2 - y1;
        return (dx * dx + dy * dy) <= (kVisionRange * kVisionRange);
    }
    
    // Check if beyond leash distance from spawn
    [[nodiscard]] bool IsBeyondLeash(int currentX, int currentY) const noexcept {
        const int dx = currentX - spawnX;
        const int dy = currentY - spawnY;
        return (dx * dx + dy * dy) > (kLeashDistance * kLeashDistance);
    }
    
    // Remove dead entities from threat list
    void CleanupThreatList();
    
    // Check if any threat is in vision range
    [[nodiscard]] bool HasThreatInVision(int currentX, int currentY) const;
};

// Combat state for player
struct PlayerCombatState {
    bool inCombat = false;
    std::set<Enemy*> engagedEnemies;     // Enemies currently fighting player
    
    // Enter combat with an enemy
    void AddEnemy(Enemy* enemy) {
        if (enemy) {
            engagedEnemies.insert(enemy);
            inCombat = true;
        }
    }
    
    // Remove enemy from combat (died or disengaged)
    void RemoveEnemy(Enemy* enemy) {
        engagedEnemies.erase(enemy);
        if (engagedEnemies.empty()) {
            inCombat = false;
        }
    }
    
    // Check if in combat
    [[nodiscard]] bool IsInCombat() const noexcept {
        return inCombat && !engagedEnemies.empty();
    }
    
    // Reset combat state
    void Reset() noexcept {
        inCombat = false;
        engagedEnemies.clear();
    }
    
    // Remove dead enemies
    void CleanupDeadEnemies();
};
