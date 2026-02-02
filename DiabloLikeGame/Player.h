#pragma once

#include "Entity.h"
#include "Combat/CombatState.h"
#include <vector>
#include <random>

// Forward declarations
class Map;
class OccupancyMap;
class Enemy;

// Player entity with movement and pathfinding
class Player : public Entity {
public:
    Player() = default;
    ~Player() override = default;

    // Initialize player at a position with health (default 100)
    void Init(int tileX, int tileY, int health = 100);

    // Update player movement (with occupancy tracking and path re-planning)
    void Update(float deltaTime, const Map& map, OccupancyMap& occupancy);

    // Movement commands (returns true if movement was successful)
    [[nodiscard]] bool MoveInDirection(int dx, int dy, const Map& map, OccupancyMap& occupancy);
    
    // Set path for click-to-move with destination for re-planning
    void SetPathToDestination(int destX, int destY, const Map& map, OccupancyMap& occupancy);
    
    // Clear current path (stop moving)
    void ClearPath() noexcept;
    
    // Attack action - returns true if punch was initiated
    bool TryPunch();
    
    // Process punch hit detection (called when punch is at peak)
    // Returns pointer to hit enemy, or nullptr if no hit
    Enemy* ProcessPunchHit(std::vector<Enemy>& enemies, std::mt19937& rng);
    
    // Combat state
    [[nodiscard]] const PlayerCombatState& GetCombatState() const noexcept { return m_combatState; }
    [[nodiscard]] PlayerCombatState& GetCombatState() noexcept { return m_combatState; }
    [[nodiscard]] bool IsInCombat() const noexcept { return m_combatState.IsInCombat(); }
    void UpdateCombatState();  // Call each frame to cleanup dead enemies

    // Player-specific getters
    [[nodiscard]] bool IsMoving() const noexcept { return m_isMoving; }
    [[nodiscard]] const std::vector<Vector2>& GetPath() const noexcept { return m_path; }
    [[nodiscard]] size_t GetPathIndex() const noexcept { return m_pathIndex; }
    [[nodiscard]] bool HasDestination() const noexcept { return m_hasDestination; }
    
    // Combat stats
    [[nodiscard]] float GetBaseAttack() const noexcept { return m_baseAttack; }
    void SetBaseAttack(float attack) noexcept { m_baseAttack = attack; }
    void SetCritChance(float chance) noexcept { m_critChance = chance; }
    void SetCritMultiplier(float mult) noexcept { m_critMultiplier = mult; }

    // Configuration
    void SetMoveSpeed(float speed) noexcept { m_moveSpeed = speed; }
    [[nodiscard]] float GetMoveSpeed() const noexcept { return m_moveSpeed; }

private:
    // Calculate speed multiplier for current movement (diagonal vs orthogonal)
    [[nodiscard]] float GetCurrentSpeedMultiplier() const noexcept;
    
    // Try to re-plan path to destination
    bool TryReplanPath(const Map& map, OccupancyMap& occupancy);
    
    // Calculate actual damage with random variation
    [[nodiscard]] int CalculateDamage(std::mt19937& rng) const;

    // Previous tile position (to determine movement direction)
    int m_prevTileX{};
    int m_prevTileY{};

    // Movement
    float m_moveSpeed{5.0f};
    bool m_isMoving{};
    bool m_isDiagonalMove{};
    
    // Path for click-to-move
    std::vector<Vector2> m_path{};
    size_t m_pathIndex{};
    
    // Destination for re-planning
    int m_destX{};
    int m_destY{};
    bool m_hasDestination{false};
    
    // Combat stats
    float m_baseAttack{20.0f};        // Player base attack
    float m_critChance{0.10f};        // 10% crit chance
    float m_critMultiplier{2.0f};     // 2x damage on crit
    
    // Combat state
    PlayerCombatState m_combatState{};
};
