#pragma once

#include "Entity.h"
#include <vector>

// Forward declarations
class Map;
class OccupancyMap;

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

    // Player-specific getters
    [[nodiscard]] bool IsMoving() const noexcept { return m_isMoving; }
    [[nodiscard]] const std::vector<Vector2>& GetPath() const noexcept { return m_path; }
    [[nodiscard]] size_t GetPathIndex() const noexcept { return m_pathIndex; }
    [[nodiscard]] bool HasDestination() const noexcept { return m_hasDestination; }

    // Configuration
    void SetMoveSpeed(float speed) noexcept { m_moveSpeed = speed; }
    [[nodiscard]] float GetMoveSpeed() const noexcept { return m_moveSpeed; }

private:
    // Calculate speed multiplier for current movement (diagonal vs orthogonal)
    [[nodiscard]] float GetCurrentSpeedMultiplier() const noexcept;
    
    // Try to re-plan path to destination
    bool TryReplanPath(const Map& map, OccupancyMap& occupancy);

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
};
