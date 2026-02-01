#pragma once

#include "raylib.h"
#include <vector>

// Forward declarations
class Map;
class OccupancyMap;

// Pathfinder - A* pathfinding algorithm
class Pathfinder {
public:
    // Find path from start to end (8-directional movement)
    // Returns empty vector if no path found
    [[nodiscard]] static std::vector<Vector2> FindPath(
        int startX, int startY,
        int endX, int endY,
        const Map& map
    );
    
    // Find path avoiding occupied tiles (for dynamic obstacle avoidance)
    [[nodiscard]] static std::vector<Vector2> FindPath(
        int startX, int startY,
        int endX, int endY,
        const Map& map,
        const OccupancyMap& occupancy
    );
    
    // Check if a tile is walkable
    [[nodiscard]] static bool IsTileWalkable(const Map& map, int x, int y) noexcept;
    
    // Check if a tile is walkable and not occupied
    [[nodiscard]] static bool IsTileWalkable(const Map& map, const OccupancyMap& occupancy, int x, int y) noexcept;
    
    // Movement costs (geometric correct values)
    static constexpr float ORTHOGONAL_COST = 1.0f;
    static constexpr float DIAGONAL_COST = 1.41421356f;  // ¡Ì2
};
