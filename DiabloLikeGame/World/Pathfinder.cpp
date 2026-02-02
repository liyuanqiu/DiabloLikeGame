#include "Pathfinder.h"
#include "Common/Map.h"
#include "OccupancyMap.h"
#include <cmath>
#include <algorithm>

// Static direction arrays definition
constexpr int Pathfinder::kDirX[];
constexpr int Pathfinder::kDirY[];
constexpr float Pathfinder::kDirCosts[];

Pathfinder& Pathfinder::Instance()
{
    // Thread-local to avoid contention in multi-threaded scenarios
    thread_local Pathfinder instance;
    return instance;
}

Pathfinder::Pathfinder()
{
    // Pre-allocate for typical path lengths
    ReserveCapacity(500);
}

void Pathfinder::ReserveCapacity(size_t estimatedNodes)
{
    m_openHeap.reserve(estimatedNodes);
    m_openSet.reserve(estimatedNodes);
    m_closedSet.reserve(estimatedNodes);
    m_gScores.reserve(estimatedNodes);
    m_cameFrom.reserve(estimatedNodes);
    m_resultPath.reserve(100);
}

void Pathfinder::ClearBuffers()
{
    m_openHeap.clear();
    m_openSet.clear();
    m_closedSet.clear();
    m_gScores.clear();
    m_cameFrom.clear();
    m_resultPath.clear();
}

float Pathfinder::Heuristic(int x1, int y1, int x2, int y2) noexcept
{
    const int dx = std::abs(x2 - x1);
    const int dy = std::abs(y2 - y1);
    // Chebyshev with diagonal cost adjustment
    return static_cast<float>(std::max(dx, dy)) + 
           (DIAGONAL_COST - ORTHOGONAL_COST) * static_cast<float>(std::min(dx, dy));
}

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

template<typename WalkableCheck>
std::vector<Vector2> Pathfinder::FindPathImpl(int startX, int startY, int endX, int endY,
                                               const Map& map, WalkableCheck&& isWalkable)
{
    // Clear previous data (doesn't deallocate)
    ClearBuffers();
    
    // Early exit if start or end is invalid
    // Note: Use base IsTileWalkable for start/end (caller's tile is occupied by them,
    // and target might be occupied by the entity we're chasing)
    if (!IsTileWalkable(map, startX, startY) || !IsTileWalkable(map, endX, endY)) {
        return {};
    }
    
    // Early exit if already at destination
    if (startX == endX && startY == endY) {
        return {};
    }
    
    // Initialize start node
    const float startH = Heuristic(startX, startY, endX, endY);
    m_openHeap.push_back({startX, startY, 0.0f, startH});
    std::push_heap(m_openHeap.begin(), m_openHeap.end(), std::greater<Node>());
    
    const uint64_t startKey = MakeKey(startX, startY);
    const uint64_t endKey = MakeKey(endX, endY);
    m_openSet.insert(startKey);
    m_gScores[startKey] = 0.0f;
    
    while (!m_openHeap.empty()) {
        // Pop minimum f-score node
        std::pop_heap(m_openHeap.begin(), m_openHeap.end(), std::greater<Node>());
        Node current = m_openHeap.back();
        m_openHeap.pop_back();
        
        const uint64_t currentKey = MakeKey(current.x, current.y);
        
        // Skip if already processed (stale entry)
        if (m_closedSet.count(currentKey)) continue;
        
        // Found the goal - reconstruct path
        if (current.x == endX && current.y == endY) {
            // Build path from goal back to start (excluding start)
            std::vector<Vector2> path;
            uint64_t key = currentKey;
            while (m_cameFrom.count(key)) {
                const int px = static_cast<int32_t>(key >> 32);
                const int py = static_cast<int32_t>(key & 0xFFFFFFFF);
                path.push_back({static_cast<float>(px), static_cast<float>(py)});
                key = m_cameFrom[key];
            }
            std::reverse(path.begin(), path.end());
            return path;
        }
        
        m_closedSet.insert(currentKey);
        m_openSet.erase(currentKey);
        
        // Explore neighbors
        for (int i = 0; i < 8; ++i) {
            const int nx = current.x + kDirX[i];
            const int ny = current.y + kDirY[i];
            const uint64_t neighborKey = MakeKey(nx, ny);
            
            // Skip if already processed
            if (m_closedSet.count(neighborKey)) continue;
            
            // Check walkability - allow destination even if occupied (we want to get close to it)
            const bool isDestination = (neighborKey == endKey);
            if (!isDestination && !isWalkable(nx, ny)) continue;
            if (isDestination && !IsTileWalkable(map, nx, ny)) continue;
            
            // For diagonal moves, check corner cutting
            if (i % 2 == 1) {  // Diagonal directions (1, 3, 5, 7)
                if (!IsTileWalkable(map, current.x + kDirX[i], current.y) ||
                    !IsTileWalkable(map, current.x, current.y + kDirY[i])) {
                    continue;
                }
            }
            
            const float tentativeG = current.g + kDirCosts[i];
            
            auto gIt = m_gScores.find(neighborKey);
            if (gIt == m_gScores.end() || tentativeG < gIt->second) {
                // This is a better path
                m_gScores[neighborKey] = tentativeG;
                m_cameFrom[neighborKey] = currentKey;
                
                const float h = Heuristic(nx, ny, endX, endY);
                m_openHeap.push_back({nx, ny, tentativeG, tentativeG + h});
                std::push_heap(m_openHeap.begin(), m_openHeap.end(), std::greater<Node>());
                m_openSet.insert(neighborKey);
            }
        }
    }
    
    return {};  // No path found
}

std::vector<Vector2> Pathfinder::FindPathInternal(int startX, int startY, int endX, int endY, const Map& map)
{
    return FindPathImpl(startX, startY, endX, endY, map, 
        [&map](int x, int y) { return IsTileWalkable(map, x, y); });
}

std::vector<Vector2> Pathfinder::FindPathInternal(int startX, int startY, int endX, int endY, 
                                           const Map& map, const OccupancyMap& occupancy)
{
    return FindPathImpl(startX, startY, endX, endY, map,
        [&map, &occupancy](int x, int y) { return IsTileWalkable(map, occupancy, x, y); });
}
