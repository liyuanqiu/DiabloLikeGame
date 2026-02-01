#include "Player.h"
#include "Map.h"
#include "World/Pathfinder.h"
#include <cmath>

void Player::Init(int tileX, int tileY)
{
    m_tileX = tileX;
    m_tileY = tileY;
    m_prevTileX = tileX;
    m_prevTileY = tileY;
    m_renderX = static_cast<float>(tileX);
    m_renderY = static_cast<float>(tileY);
    m_isMoving = false;
    m_isDiagonalMove = false;
    m_path.clear();
    m_pathIndex = 0;
}

float Player::GetCurrentSpeedMultiplier() const noexcept
{
    // Grid-based timing:
    // - Orthogonal move (1 grid step): 1 time unit
    // - Diagonal move (2 grid steps): 2 time units
    // sqrt(2) / 2 = 1 / sqrt(2) ¡Ö 0.70710678f
    return m_isDiagonalMove ? 0.70710678f : 1.0f;
}

bool Player::MoveInDirection(int dx, int dy, const Map& map)
{
    if (m_isMoving) return false;
    
    const int newX = m_tileX + dx;
    const int newY = m_tileY + dy;
    
    if (Pathfinder::IsTileWalkable(map, newX, newY)) {
        m_prevTileX = m_tileX;
        m_prevTileY = m_tileY;
        m_tileX = newX;
        m_tileY = newY;
        m_isMoving = true;
        m_isDiagonalMove = (dx != 0 && dy != 0);
        m_path.clear();
        return true;
    }
    return false;
}

void Player::SetPath(std::vector<Vector2> path)
{
    m_path = std::move(path);
    m_pathIndex = 0;
    
    if (!m_path.empty()) {
        // Initialize first waypoint
        const auto& firstTarget = m_path[0];
        const int newTileX = static_cast<int>(firstTarget.x);
        const int newTileY = static_cast<int>(firstTarget.y);
        
        m_prevTileX = m_tileX;
        m_prevTileY = m_tileY;
        m_tileX = newTileX;
        m_tileY = newTileY;
        m_isDiagonalMove = (newTileX != m_prevTileX && newTileY != m_prevTileY);
        m_isMoving = true;
    }
}

void Player::ClearPath() noexcept
{
    m_path.clear();
    m_pathIndex = 0;
}

void Player::Update(float deltaTime)
{
    const float targetX = static_cast<float>(m_tileX);
    const float targetY = static_cast<float>(m_tileY);
    const float dx = targetX - m_renderX;
    const float dy = targetY - m_renderY;
    const float distSq = dx * dx + dy * dy;
    
    constexpr float kArrivalThresholdSq = 0.01f * 0.01f;
    constexpr float kPathAdvanceThresholdSq = 0.05f * 0.05f;
    
    // Check if we've arrived at current target
    if (distSq <= kArrivalThresholdSq) {
        m_renderX = targetX;
        m_renderY = targetY;
        
        // If following a path, advance to next waypoint
        if (!m_path.empty() && m_pathIndex < m_path.size()) {
            ++m_pathIndex;
            
            if (m_pathIndex < m_path.size()) {
                // Set up next waypoint
                const auto& nextTarget = m_path[m_pathIndex];
                const int newTileX = static_cast<int>(nextTarget.x);
                const int newTileY = static_cast<int>(nextTarget.y);
                
                m_prevTileX = m_tileX;
                m_prevTileY = m_tileY;
                m_tileX = newTileX;
                m_tileY = newTileY;
                m_isDiagonalMove = (newTileX != m_prevTileX && newTileY != m_prevTileY);
            } else {
                // Path complete
                m_path.clear();
                m_isMoving = false;
                m_isDiagonalMove = false;
            }
        } else if (m_path.empty()) {
            m_isMoving = false;
            m_isDiagonalMove = false;
        }
        return;
    }
    
    // Move toward target
    const float dist = std::sqrt(distSq);
    const float effectiveSpeed = m_moveSpeed * GetCurrentSpeedMultiplier();
    const float moveAmount = effectiveSpeed * deltaTime;
    
    if (moveAmount >= dist) {
        m_renderX = targetX;
        m_renderY = targetY;
    } else {
        const float invDist = 1.0f / dist;
        m_renderX += dx * invDist * moveAmount;
        m_renderY += dy * invDist * moveAmount;
    }
}
