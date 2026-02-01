#pragma once

#include "Entity.h"
#include <random>

// Forward declarations
class Map;
class OccupancyMap;

// Enemy entity with wandering behavior
class Enemy : public Entity {
public:
    Enemy() = default;
    Enemy(int tileX, int tileY, int health, std::mt19937& rng);
    ~Enemy() override = default;
    
    // Update enemy behavior (wandering)
    void Update(float deltaTime, const Map& map, OccupancyMap& occupancy, std::mt19937& rng);
    
    // Wandering configuration
    void SetWanderRadius(int radius) noexcept { m_wanderRadius = radius; }
    [[nodiscard]] int GetWanderRadius() const noexcept { return m_wanderRadius; }
    void SetMoveSpeed(float speed) noexcept { m_moveSpeed = speed; }

private:
    // Try to move one step in a random direction
    bool TryMoveOneStep(const Map& map, OccupancyMap& occupancy, std::mt19937& rng);
    
    // Get speed multiplier for diagonal vs orthogonal movement
    [[nodiscard]] float GetCurrentSpeedMultiplier() const noexcept;
    
    // Spawn position (center of wander area)
    int m_spawnX{};
    int m_spawnY{};
    
    // Previous position (for diagonal detection)
    int m_prevTileX{};
    int m_prevTileY{};
    
    // Wandering state
    int m_wanderRadius{5};        // Max distance from spawn point
    float m_pauseTimer{0.0f};     // Time remaining before next move
    bool m_isMoving{false};       // Currently moving to next tile
    bool m_isDiagonalMove{false}; // Current move is diagonal
    
    // Movement speed (tiles per second)
    float m_moveSpeed{3.0f};
};
