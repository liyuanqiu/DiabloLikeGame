#pragma once

#include "../Config/CombatConfig.h"
#include <array>
#include <algorithm>
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

// Fixed-size threat list to avoid dynamic allocation
// Most combat scenarios have few threats (typically 1-3)
template<typename T, size_t MaxSize>
class SmallSet {
public:
    void clear() noexcept { m_size = 0; }
    [[nodiscard]] size_t size() const noexcept { return m_size; }
    [[nodiscard]] bool empty() const noexcept { return m_size == 0; }
    
    void insert(T value) {
        if (m_size >= MaxSize) return;  // Silently ignore overflow
        // Check for duplicates
        for (size_t i = 0; i < m_size; ++i) {
            if (m_data[i] == value) return;
        }
        m_data[m_size++] = value;
    }
    
    void erase(T value) noexcept {
        for (size_t i = 0; i < m_size; ++i) {
            if (m_data[i] == value) {
                m_data[i] = m_data[--m_size];
                return;
            }
        }
    }
    
    [[nodiscard]] bool contains(T value) const noexcept {
        for (size_t i = 0; i < m_size; ++i) {
            if (m_data[i] == value) return true;
        }
        return false;
    }
    
    // Iterator support
    T* begin() noexcept { return m_data.data(); }
    T* end() noexcept { return m_data.data() + m_size; }
    const T* begin() const noexcept { return m_data.data(); }
    const T* end() const noexcept { return m_data.data() + m_size; }

private:
    std::array<T, MaxSize> m_data{};
    size_t m_size = 0;
};

// Combat state for enemies
struct EnemyCombatState {
    bool inCombat = false;
    CombatBehavior behavior = CombatBehavior::Wandering;
    
    // Timing
    float combatTimer = 0.0f;           // Time since last successful action
    float lastDamageReceivedTime = 0.0f; // Time since last took damage
    
    // Threat tracking (fixed-size to avoid allocation)
    SmallSet<Entity*, 8> threatList;    // All entities that have damaged us
    Entity* currentTarget = nullptr;     // Current chase/attack target
    
    // Spawn point for returning
    int spawnX = 0;
    int spawnY = 0;
    
    // Configuration (using CombatConfig defaults)
    static constexpr float kCombatTimeout = CombatConfig::Enemy::kCombatTimeout;
    static constexpr int kLeashDistance = CombatConfig::Enemy::kLeashDistance;
    static constexpr int kVisionRange = CombatConfig::Enemy::kVisionRange;
    
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
    SmallSet<Enemy*, 16> engagedEnemies;  // Enemies currently fighting player
    
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
