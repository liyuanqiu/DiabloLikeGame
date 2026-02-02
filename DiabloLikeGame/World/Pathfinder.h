#pragma once

#include "raylib.h"
#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <cstdint>

// Forward declarations
class Map;
class OccupancyMap;

// A* pathfinding with reusable memory buffers
// Uses instance-based design to avoid per-call memory allocation
class Pathfinder {
public:
    // Get thread-local instance for pathfinding (avoids allocation per call)
    static Pathfinder& Instance();
    
    // Static convenience methods (use Instance() internally)
    // These maintain backward compatibility with existing code
    [[nodiscard]] static std::vector<Vector2> FindPath(
        int startX, int startY, int endX, int endY, const Map& map) {
        return Instance().FindPathInternal(startX, startY, endX, endY, map);
    }
    
    [[nodiscard]] static std::vector<Vector2> FindPath(
        int startX, int startY, int endX, int endY, 
        const Map& map, const OccupancyMap& occupancy) {
        return Instance().FindPathInternal(startX, startY, endX, endY, map, occupancy);
    }
    
    // Check if a tile is walkable
    [[nodiscard]] static bool IsTileWalkable(const Map& map, int x, int y) noexcept;
    
    // Check if a tile is walkable and not occupied
    [[nodiscard]] static bool IsTileWalkable(const Map& map, const OccupancyMap& occupancy, int x, int y) noexcept;
    
    // Movement costs (geometric correct values)
    static constexpr float ORTHOGONAL_COST = 1.0f;
    static constexpr float DIAGONAL_COST = 1.41421356f;  // sqrt(2)

private:
    Pathfinder();
    
    // Internal pathfinding methods
    [[nodiscard]] std::vector<Vector2> FindPathInternal(
        int startX, int startY, int endX, int endY, const Map& map);
    [[nodiscard]] std::vector<Vector2> FindPathInternal(
        int startX, int startY, int endX, int endY, 
        const Map& map, const OccupancyMap& occupancy);
    
    // Pre-allocate capacity for typical pathfinding
    void ReserveCapacity(size_t estimatedNodes);
    
    // Clear buffers for reuse (doesn't deallocate)
    void ClearBuffers();
    
    // A* node structure
    struct Node {
        int x, y;
        float g, f;
        bool operator>(const Node& other) const { return f > other.f; }
    };
    
    // Compact key for hash maps
    [[nodiscard]] static uint64_t MakeKey(int x, int y) noexcept {
        return (static_cast<uint64_t>(static_cast<uint32_t>(x)) << 32) | 
               static_cast<uint64_t>(static_cast<uint32_t>(y));
    }
    
    // Chebyshev distance heuristic
    [[nodiscard]] static float Heuristic(int x1, int y1, int x2, int y2) noexcept;
    
    // Core A* implementation (template to handle occupancy check difference)
    template<typename WalkableCheck>
    std::vector<Vector2> FindPathImpl(int startX, int startY, int endX, int endY,
                                       const Map& map, WalkableCheck&& isWalkable);
    
    // Reusable containers (cleared but not reallocated between calls)
    std::vector<Node> m_openHeap;
    std::unordered_set<uint64_t> m_openSet;
    std::unordered_set<uint64_t> m_closedSet;
    std::unordered_map<uint64_t, float> m_gScores;
    std::unordered_map<uint64_t, uint64_t> m_cameFrom;
    std::vector<Vector2> m_resultPath;
    
    // Direction offsets for 8-directional movement
    static constexpr int kDirX[] = {1, 1, 0, -1, -1, -1, 0, 1};
    static constexpr int kDirY[] = {0, 1, 1, 1, 0, -1, -1, -1};
    static constexpr float kDirCosts[] = {
        ORTHOGONAL_COST, DIAGONAL_COST, ORTHOGONAL_COST, DIAGONAL_COST,
        ORTHOGONAL_COST, DIAGONAL_COST, ORTHOGONAL_COST, DIAGONAL_COST
    };
};
