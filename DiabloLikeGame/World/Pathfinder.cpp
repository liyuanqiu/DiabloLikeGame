#include "Pathfinder.h"
#include "Common/Map.h"
#include "OccupancyMap.h"
#include <cmath>
#include <cstdint>
#include <queue>
#include <unordered_set>
#include <unordered_map>

bool Pathfinder::IsTileWalkable(const Map& map, int x, int y) noexcept
{
    if (x < 0 || x >= map.GetWidth() || y < 0 || y >= map.GetHeight()) {
        return false;
    }
    return map.GetTileUnchecked(x, y) == TileType::Floor;
}

bool Pathfinder::IsTileWalkable(const Map& map, const OccupancyMap& occupancy, int x, int y) noexcept
{
    if (!IsTileWalkable(map, x, y)) {
        return false;
    }
    return !occupancy.IsOccupied(x, y);
}

std::vector<Vector2> Pathfinder::FindPath(int startX, int startY, int endX, int endY, const Map& map)
{
    // Early exit if start or end is invalid
    if (!IsTileWalkable(map, startX, startY) || !IsTileWalkable(map, endX, endY)) {
        return {};
    }
    
    // Early exit if already at destination
    if (startX == endX && startY == endY) {
        return {};
    }

    struct Node {
        int x, y;
        float g, f;  // g = cost from start, f = g + h
        
        // For priority queue (min-heap by f value)
        bool operator>(const Node& other) const { return f > other.f; }
    };
    
    // Chebyshev distance heuristic (for 8-directional with diagonal cost ¡Ì2)
    auto heuristic = [](int x1, int y1, int x2, int y2) -> float {
        const int dx = std::abs(x2 - x1);
        const int dy = std::abs(y2 - y1);
        // D + (D2 - D) * min(dx, dy) where D=1, D2=¡Ì2
        return static_cast<float>(std::max(dx, dy)) + 
               (DIAGONAL_COST - ORTHOGONAL_COST) * static_cast<float>(std::min(dx, dy));
    };
    
    // Compact key for hash maps (handles negative coordinates correctly)
    auto makeKey = [](int x, int y) -> uint64_t {
        // Use unsigned conversion to preserve bit pattern for negative numbers
        return (static_cast<uint64_t>(static_cast<uint32_t>(x)) << 32) | 
               static_cast<uint64_t>(static_cast<uint32_t>(y));
    };
    
    // Priority queue for open list (min-heap)
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openQueue;
    std::unordered_set<uint64_t> openSet;    // Track what's in the queue
    std::unordered_set<uint64_t> closedSet;  // Already processed
    std::unordered_map<uint64_t, float> gScores;  // Best g score for each node
    std::unordered_map<uint64_t, uint64_t> cameFrom;  // Parent tracking
    
    // Pre-allocate based on expected path length (Manhattan distance * 2)
    const size_t estimatedNodes = static_cast<size_t>(
        (std::abs(endX - startX) + std::abs(endY - startY)) * 2 + 100);
    openSet.reserve(estimatedNodes);
    closedSet.reserve(estimatedNodes);
    gScores.reserve(estimatedNodes);
    cameFrom.reserve(estimatedNodes);
    
    // Initialize start node
    const float startH = heuristic(startX, startY, endX, endY);
    openQueue.push({startX, startY, 0.0f, startH});
    openSet.insert(makeKey(startX, startY));
    gScores[makeKey(startX, startY)] = 0.0f;
    
    // Direction offsets (8-directional): E, SE, S, SW, W, NW, N, NE
    constexpr int dx[] = {1, 1, 0, -1, -1, -1, 0, 1};
    constexpr int dy[] = {0, 1, 1, 1, 0, -1, -1, -1};
    constexpr float costs[] = {
        ORTHOGONAL_COST, DIAGONAL_COST, ORTHOGONAL_COST, DIAGONAL_COST,
        ORTHOGONAL_COST, DIAGONAL_COST, ORTHOGONAL_COST, DIAGONAL_COST
    };
    
    while (!openQueue.empty()) {
        Node current = openQueue.top();
        openQueue.pop();
        
        const uint64_t currentKey = makeKey(current.x, current.y);
        
        // Skip if already processed (stale entry in queue)
        if (closedSet.count(currentKey)) continue;
        
        // Found the goal
        if (current.x == endX && current.y == endY) {
            // Reconstruct path (excluding start node)
            std::vector<Vector2> path;
            uint64_t key = currentKey;
            
            while (cameFrom.count(key)) {
                const int px = static_cast<int32_t>(key >> 32);
                const int py = static_cast<int32_t>(key & 0xFFFFFFFF);
                path.push_back({static_cast<float>(px), static_cast<float>(py)});
                key = cameFrom[key];
            }
            
            std::reverse(path.begin(), path.end());
            return path;
        }
        
        closedSet.insert(currentKey);
        openSet.erase(currentKey);
        
        // Explore neighbors
        for (int i = 0; i < 8; ++i) {
            const int nx = current.x + dx[i];
            const int ny = current.y + dy[i];
            const uint64_t neighborKey = makeKey(nx, ny);
            
            // Skip if already processed or not walkable
            if (closedSet.count(neighborKey)) continue;
            if (!IsTileWalkable(map, nx, ny)) continue;
            
            // For diagonal moves, check if we can actually move diagonally
            // (avoid cutting corners through walls)
            if (i % 2 == 1) {  // Diagonal directions
                if (!IsTileWalkable(map, current.x + dx[i], current.y) ||
                    !IsTileWalkable(map, current.x, current.y + dy[i])) {
                    continue;
                }
            }
            
            const float tentativeG = current.g + costs[i];
            
            
            auto gIt = gScores.find(neighborKey);
            if (gIt == gScores.end() || tentativeG < gIt->second) {
                // This is a better path
                gScores[neighborKey] = tentativeG;
                cameFrom[neighborKey] = currentKey;
                
                const float h = heuristic(nx, ny, endX, endY);
                openQueue.push({nx, ny, tentativeG, tentativeG + h});
                openSet.insert(neighborKey);
            }
        }
    }
    
    return {};  // No path found
}

std::vector<Vector2> Pathfinder::FindPath(
    int startX, int startY, int endX, int endY, 
    const Map& map, const OccupancyMap& occupancy)
{
    // Early exit if start or end is invalid (destination can be occupied - we want to get close)
    if (!IsTileWalkable(map, startX, startY) || !IsTileWalkable(map, endX, endY)) {
        return {};
    }
    
    // Early exit if already at destination
    if (startX == endX && startY == endY) {
        return {};
    }

    struct Node {
        int x, y;
        float g, f;
        bool operator>(const Node& other) const { return f > other.f; }
    };
    
    auto heuristic = [](int x1, int y1, int x2, int y2) -> float {
        const int dx = std::abs(x2 - x1);
        const int dy = std::abs(y2 - y1);
        return static_cast<float>(std::max(dx, dy)) + 
               (DIAGONAL_COST - ORTHOGONAL_COST) * static_cast<float>(std::min(dx, dy));
    };
    
    auto makeKey = [](int x, int y) -> uint64_t {
        return (static_cast<uint64_t>(static_cast<uint32_t>(x)) << 32) | 
               static_cast<uint64_t>(static_cast<uint32_t>(y));
    };
    
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openQueue;
    std::unordered_set<uint64_t> openSet;
    std::unordered_set<uint64_t> closedSet;
    std::unordered_map<uint64_t, float> gScores;
    std::unordered_map<uint64_t, uint64_t> cameFrom;
    
    const size_t estimatedNodes = static_cast<size_t>(
        (std::abs(endX - startX) + std::abs(endY - startY)) * 2 + 100);
    openSet.reserve(estimatedNodes);
    closedSet.reserve(estimatedNodes);
    gScores.reserve(estimatedNodes);
    cameFrom.reserve(estimatedNodes);
    
    const float startH = heuristic(startX, startY, endX, endY);
    openQueue.push({startX, startY, 0.0f, startH});
    openSet.insert(makeKey(startX, startY));
    gScores[makeKey(startX, startY)] = 0.0f;
    
    constexpr int dx[] = {1, 1, 0, -1, -1, -1, 0, 1};
    constexpr int dy[] = {0, 1, 1, 1, 0, -1, -1, -1};
    constexpr float costs[] = {
        ORTHOGONAL_COST, DIAGONAL_COST, ORTHOGONAL_COST, DIAGONAL_COST,
        ORTHOGONAL_COST, DIAGONAL_COST, ORTHOGONAL_COST, DIAGONAL_COST
    };
    
    while (!openQueue.empty()) {
        Node current = openQueue.top();
        openQueue.pop();
        
        const uint64_t currentKey = makeKey(current.x, current.y);
        
        if (closedSet.count(currentKey)) continue;
        
        if (current.x == endX && current.y == endY) {
            std::vector<Vector2> path;
            uint64_t key = currentKey;
            
            while (cameFrom.count(key)) {
                const int px = static_cast<int32_t>(key >> 32);
                const int py = static_cast<int32_t>(key & 0xFFFFFFFF);
                path.push_back({static_cast<float>(px), static_cast<float>(py)});
                key = cameFrom[key];
            }
            
            std::reverse(path.begin(), path.end());
            return path;
        }
        
        closedSet.insert(currentKey);
        openSet.erase(currentKey);
        
        for (int i = 0; i < 8; ++i) {
            const int nx = current.x + dx[i];
            const int ny = current.y + dy[i];
            const uint64_t neighborKey = makeKey(nx, ny);
            
            if (closedSet.count(neighborKey)) continue;
            
            // Use occupancy-aware walkability check
            // But allow destination tile even if occupied (we want to path to it)
            const bool isDestination = (nx == endX && ny == endY);
            if (!isDestination && !IsTileWalkable(map, occupancy, nx, ny)) continue;
            if (isDestination && !IsTileWalkable(map, nx, ny)) continue;
            
            // Check diagonal corner cutting
            if (i % 2 == 1) {
                if (!IsTileWalkable(map, current.x + dx[i], current.y) ||
                    !IsTileWalkable(map, current.x, current.y + dy[i])) {
                    continue;
                }
            }
            
            const float tentativeG = current.g + costs[i];
            
            auto gIt = gScores.find(neighborKey);
            if (gIt == gScores.end() || tentativeG < gIt->second) {
                gScores[neighborKey] = tentativeG;
                cameFrom[neighborKey] = currentKey;
                
                const float h = heuristic(nx, ny, endX, endY);
                openQueue.push({nx, ny, tentativeG, tentativeG + h});
                openSet.insert(neighborKey);
            }
        }
    }
    
    return {};
}
