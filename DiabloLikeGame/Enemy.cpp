#include "Enemy.h"
#include "Map.h"
#include "World/Pathfinder.h"
#include "World/OccupancyMap.h"
#include <cmath>
#include <algorithm>

Enemy::Enemy(int tileX, int tileY, int health, std::mt19937& rng)
    : Entity(tileX, tileY)
    , m_spawnX(tileX)
    , m_spawnY(tileY)
    , m_prevTileX(tileX)
    , m_prevTileY(tileY)
{
    SetHealth(health, health);
    
    // Random initial pause (1.0 - 4.0 seconds) - don't move immediately
    std::uniform_real_distribution<float> pauseDist(1.0f, 4.0f);
    m_pauseTimer = pauseDist(rng);
}

float Enemy::GetCurrentSpeedMultiplier() const noexcept
{
    // Same as player: diagonal moves take longer
    // sqrt(2) / 2 = 1 / sqrt(2) ¡Ö 0.70710678f
    return m_isDiagonalMove ? 0.70710678f : 1.0f;
}

void Enemy::Update(float deltaTime, const Map& map, OccupancyMap& occupancy, std::mt19937& rng)
{
    if (!IsAlive()) return;
    
    if (m_isMoving) {
        // Moving toward target tile (same logic as Player)
        const float targetX = static_cast<float>(GetTileX());
        const float targetY = static_cast<float>(GetTileY());
        const float dx = targetX - GetRenderX();
        const float dy = targetY - GetRenderY();
        const float distSq = dx * dx + dy * dy;
        
        constexpr float kArrivalThresholdSq = 0.01f * 0.01f;
        
        if (distSq <= kArrivalThresholdSq) {
            // Arrived at destination
            SetRenderPosition(targetX, targetY);
            m_isMoving = false;
            m_isDiagonalMove = false;
            
            // Random pause before next move (1.5 - 4.0 seconds)
            std::uniform_real_distribution<float> pauseDist(1.5f, 4.0f);
            m_pauseTimer = pauseDist(rng);
        } else {
            // Move toward target
            const float dist = std::sqrt(distSq);
            const float effectiveSpeed = m_moveSpeed * GetCurrentSpeedMultiplier();
            const float moveAmount = effectiveSpeed * deltaTime;
            
            if (moveAmount >= dist) {
                SetRenderPosition(targetX, targetY);
            } else {
                const float invDist = 1.0f / dist;
                SetRenderPosition(
                    GetRenderX() + dx * invDist * moveAmount,
                    GetRenderY() + dy * invDist * moveAmount
                );
            }
        }
    } else {
        // Waiting/paused
        m_pauseTimer -= deltaTime;
        
        if (m_pauseTimer <= 0.0f) {
            // Try to move one step
            if (!TryMoveOneStep(map, occupancy, rng)) {
                // Couldn't move, wait a bit more
                std::uniform_real_distribution<float> pauseDist(0.5f, 1.5f);
                m_pauseTimer = pauseDist(rng);
            }
        }
    }
}

bool Enemy::TryMoveOneStep(const Map& map, OccupancyMap& occupancy, std::mt19937& rng)
{
    // 8 directions: N, NE, E, SE, S, SW, W, NW
    constexpr int dx[] = {0, 1, 1, 1, 0, -1, -1, -1};
    constexpr int dy[] = {-1, -1, 0, 1, 1, 1, 0, -1};
    
    // Shuffle direction order for randomness
    int directions[] = {0, 1, 2, 3, 4, 5, 6, 7};
    std::shuffle(std::begin(directions), std::end(directions), rng);
    
    const int currentX = GetTileX();
    const int currentY = GetTileY();
    
    // Try each direction in random order
    for (int dir : directions) {
        const int newX = currentX + dx[dir];
        const int newY = currentY + dy[dir];
        
        // Check if within wander radius from spawn point
        const int distFromSpawnX = newX - m_spawnX;
        const int distFromSpawnY = newY - m_spawnY;
        if (distFromSpawnX * distFromSpawnX + distFromSpawnY * distFromSpawnY > 
            m_wanderRadius * m_wanderRadius) {
            continue;
        }
        
        // Check if tile is walkable
        if (!Pathfinder::IsTileWalkable(map, newX, newY)) {
            continue;
        }
        
        // Check if tile is occupied by another entity
        if (occupancy.IsOccupied(newX, newY)) {
            continue;
        }
        
        // For diagonal moves, check corner cutting
        const bool isDiagonal = (dx[dir] != 0 && dy[dir] != 0);
        if (isDiagonal) {
            if (!Pathfinder::IsTileWalkable(map, currentX + dx[dir], currentY) ||
                !Pathfinder::IsTileWalkable(map, currentX, currentY + dy[dir])) {
                continue;
            }
        }
        
        // Valid move found - start moving
        m_prevTileX = currentX;
        m_prevTileY = currentY;
        SetTilePosition(newX, newY);
        m_isMoving = true;
        m_isDiagonalMove = isDiagonal;
        
        // Update occupancy
        occupancy.Move(currentX, currentY, newX, newY);
        return true;
    }
    
    return false;  // No valid move found
}
