#pragma once

#include "Entity.h"
#include "Config/EntityConfig.h"
#include <random>
#include <string>

// Forward declarations
class Map;
class OccupancyMap;
class Player;

// Enemy entity with wandering behavior and combat
class Enemy : public Entity {
public:
    Enemy() = default;
    Enemy(int tileX, int tileY, std::mt19937& rng);  // Default type
    Enemy(int tileX, int tileY, const EnemyTypeConfig& config, std::mt19937& rng);
    ~Enemy() override = default;
    
    // Update enemy behavior (wandering and combat)
    void Update(float deltaTime, const Map& map, OccupancyMap& occupancy, 
                std::mt19937& rng, Player* player = nullptr);
    
    // Override TakeDamage to trigger aggression
    void TakeDamage(int amount) noexcept;
    
    // Apply configuration
    void ApplyConfig(const EnemyTypeConfig& config);
    
    // Identity
    [[nodiscard]] const std::string& GetTypeId() const noexcept { return m_typeId; }
    [[nodiscard]] const std::string& GetDisplayName() const noexcept { return m_displayName; }
    
    // Wandering configuration
    void SetWanderRadius(int radius) noexcept { m_wanderRadius = radius; }
    [[nodiscard]] int GetWanderRadius() const noexcept { return m_wanderRadius; }
    void SetMoveSpeed(float speed) noexcept { m_moveSpeed = speed; }
    
    // Combat configuration
    void SetBaseAttack(float attack) noexcept { m_baseAttack = attack; }
    [[nodiscard]] float GetBaseAttack() const noexcept { return m_baseAttack; }
    
    // Aggression state
    [[nodiscard]] AggressionType GetAggressionType() const noexcept { return m_aggressionType; }
    void SetAggressionType(AggressionType type) noexcept { m_aggressionType = type; }
    [[nodiscard]] bool IsAggressive() const noexcept { return m_isAggressive; }
    void SetAggressive(bool aggressive) noexcept { m_isAggressive = aggressive; }
    
    // Visual
    [[nodiscard]] Color GetColor() const noexcept { return m_color; }

private:
    // Try to move one step in a random direction
    bool TryMoveOneStep(const Map& map, OccupancyMap& occupancy, std::mt19937& rng);
    
    // Get speed multiplier for diagonal vs orthogonal movement
    [[nodiscard]] float GetCurrentSpeedMultiplier() const noexcept;
    
    // Check if player is adjacent and try to attack
    bool TryAttackPlayer(Player* player, std::mt19937& rng);
    
    // Calculate damage with random variation
    [[nodiscard]] int CalculateDamage(std::mt19937& rng) const;
    
    // Identity
    std::string m_typeId{"goblin"};
    std::string m_displayName{"Goblin"};
    
    // Spawn position (center of wander area)
    int m_spawnX{};
    int m_spawnY{};
    
    // Previous position (for diagonal detection)
    int m_prevTileX{};
    int m_prevTileY{};
    
    // Wandering state
    int m_wanderRadius{5};
    float m_pauseTimer{0.0f};
    float m_pauseTimeMin{1.5f};
    float m_pauseTimeMax{4.0f};
    bool m_isMoving{false};
    bool m_isDiagonalMove{false};
    
    // Movement speed (tiles per second)
    float m_moveSpeed{3.0f};
    
    // Combat stats
    float m_baseAttack{10.0f};
    float m_attackVariation{0.1f};
    float m_attackCooldown{0.0f};
    float m_attackCooldownTime{1.0f};
    bool m_punchHitProcessed{false};
    
    // Aggression
    AggressionType m_aggressionType{AggressionType::Defensive};
    bool m_isAggressive{false};
    
    // Visual
    Color m_color{230, 41, 55, 255};
};
