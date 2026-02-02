#pragma once

#include "Entity.h"
#include "Config/EntityConfig.h"
#include "Combat/CombatState.h"
#include "raylib.h"
#include <random>
#include <string>
#include <vector>

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
    void TakeDamage(int amount, Entity* attacker = nullptr) noexcept;
    
    // Apply configuration
    void ApplyConfig(const EnemyTypeConfig& config);
    
    // Combat state access
    [[nodiscard]] const EnemyCombatState& GetCombatState() const noexcept { return m_combatState; }
    [[nodiscard]] EnemyCombatState& GetCombatState() noexcept { return m_combatState; }
    [[nodiscard]] bool IsInCombat() const noexcept { return m_combatState.inCombat; }
    [[nodiscard]] CombatBehavior GetCombatBehavior() const noexcept { return m_combatState.behavior; }
    
    // Notify when combat ends (for player tracking)
    void ExitCombat() noexcept;
    
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
    [[nodiscard]] Color GetColor() const noexcept { 
        if (IsHit()) return Color{255, 255, 255, 255};
        return m_color; 
    }

private:
// Behavior updates based on aggression type
void UpdatePassiveBehavior(float deltaTime, const Map& map, OccupancyMap& occupancy, 
                           std::mt19937& rng, Player* player);
void UpdateDefensiveBehavior(float deltaTime, const Map& map, OccupancyMap& occupancy, 
                             std::mt19937& rng, Player* player);
void UpdateAggressiveBehavior(float deltaTime, const Map& map, OccupancyMap& occupancy, 
                              std::mt19937& rng, Player* player);
void UpdateReturningBehavior(float deltaTime, const Map& map, OccupancyMap& occupancy, 
                             std::mt19937& rng);
void UpdateWanderingBehavior(float deltaTime, const Map& map, OccupancyMap& occupancy, 
                             std::mt19937& rng, Player* player);
    
// Try to move one step in a random direction (for wandering)
bool TryMoveOneStep(const Map& map, OccupancyMap& occupancy, std::mt19937& rng);
    
// Try to move toward a target position (uses A* pathfinding)
bool TryMoveToward(int targetX, int targetY, const Map& map, OccupancyMap& occupancy);
    
// Try to move away from a target position (for fleeing)
bool TryMoveAwayFrom(int targetX, int targetY, const Map& map, OccupancyMap& occupancy, 
                     std::mt19937& rng);
    
// Try to move toward spawn point (for returning)
bool TryMoveTowardSpawn(const Map& map, OccupancyMap& occupancy);

// Follow cached path (returns false if path is empty or blocked)
bool FollowPath(const Map& map, OccupancyMap& occupancy);

// Clear the cached path
void ClearPath() noexcept { m_path.clear(); m_pathIndex = 0; }
    
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
    
    // Pathfinding cache
    std::vector<Vector2> m_path;
    size_t m_pathIndex{0};
    int m_lastTargetX{-1};
    int m_lastTargetY{-1};
    
    // Combat stats
    float m_baseAttack{10.0f};
    float m_attackVariation{0.1f};
    float m_attackCooldown{0.0f};
    float m_attackCooldownTime{1.0f};
    
    // Aggression
    AggressionType m_aggressionType{AggressionType::Defensive};
    bool m_isAggressive{false};
    
    // Combat state
    EnemyCombatState m_combatState{};
    
    // Visual
    Color m_color{230, 41, 55, 255};
};

