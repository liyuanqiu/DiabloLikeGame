#pragma once

#include "Entity.h"
#include <vector>

// Forward declaration
class Map;

// Player entity with movement and pathfinding
class Player : public Entity {
public:
    Player() = default;
    ~Player() override = default;

    // Initialize player at a position with health (default 100)
    void Init(int tileX, int tileY, int health = 100);

    // Update player movement
    void Update(float deltaTime);

    // Movement commands (returns true if movement was successful)
    [[nodiscard]] bool MoveInDirection(int dx, int dy, const Map& map);
    
    // Set path for click-to-move (path from Pathfinder)
    void SetPath(std::vector<Vector2> path);
    
    // Clear current path (stop moving)
    void ClearPath() noexcept;

    // Player-specific getters
    [[nodiscard]] bool IsMoving() const noexcept { return m_isMoving; }
    [[nodiscard]] const std::vector<Vector2>& GetPath() const noexcept { return m_path; }
    [[nodiscard]] size_t GetPathIndex() const noexcept { return m_pathIndex; }

    // Configuration
    void SetMoveSpeed(float speed) noexcept { m_moveSpeed = speed; }
    [[nodiscard]] float GetMoveSpeed() const noexcept { return m_moveSpeed; }

private:
    // Calculate speed multiplier for current movement (diagonal vs orthogonal)
    [[nodiscard]] float GetCurrentSpeedMultiplier() const noexcept;

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
};
