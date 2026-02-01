#include "Player.h"
#include "Enemy.h"
#include "Map.h"
#include "World/Pathfinder.h"
#include "World/OccupancyMap.h"
#include <cmath>

void Player::Init(int tileX, int tileY, int health)
{
    SetTilePosition(tileX, tileY);
    SetRenderPosition(static_cast<float>(tileX), static_cast<float>(tileY));
    SetHealth(health, health);
    m_prevTileX = tileX;
    m_prevTileY = tileY;
    m_isMoving = false;
    m_isDiagonalMove = false;
    m_path.clear();
    m_pathIndex = 0;
    m_hasDestination = false;
}

float Player::GetCurrentSpeedMultiplier() const noexcept
{
    // Grid-based timing:
    // - Orthogonal move (1 grid step): 1 time unit
    // - Diagonal move (2 grid steps): 2 time units
    // sqrt(2) / 2 = 1 / sqrt(2) ≈ 0.70710678f
    return m_isDiagonalMove ? 0.70710678f : 1.0f;
}


bool Player::MoveInDirection(int dx, int dy, const Map& map, OccupancyMap& occupancy)
{
    if (m_isMoving) return false;
    
    const int oldX = GetTileX();
    const int oldY = GetTileY();
    const int newX = oldX + dx;
    const int newY = oldY + dy;
    
    // Check if walkable and not occupied by another entity
    if (Pathfinder::IsTileWalkable(map, newX, newY) && !occupancy.IsOccupied(newX, newY)) {
        m_prevTileX = oldX;
        m_prevTileY = oldY;
        SetTilePosition(newX, newY);
        m_isMoving = true;
        m_isDiagonalMove = (dx != 0 && dy != 0);
        m_path.clear();
        m_hasDestination = false;  // Manual movement clears destination
        
        // Update facing direction
        SetFacing(DirectionUtil::FromDelta(dx, dy));
        
        // Update occupancy
        occupancy.Move(oldX, oldY, newX, newY);
        return true;
    }
    return false;
}



void Player::SetPathToDestination(int destX, int destY, const Map& map, OccupancyMap& occupancy)
{
    // Store destination for re-planning
    m_destX = destX;
    m_destY = destY;
    m_hasDestination = true;
    
    // Find path to destination (avoiding occupied tiles)
    auto path = Pathfinder::FindPath(GetTileX(), GetTileY(), destX, destY, map, occupancy);
    
    m_path = std::move(path);
    m_pathIndex = 0;
    
    if (!m_path.empty()) {
        // Initialize first waypoint
        const auto& firstTarget = m_path[0];
        const int newTileX = static_cast<int>(firstTarget.x);
        const int newTileY = static_cast<int>(firstTarget.y);
        
        // Check if first tile is occupied - try re-plan later
        if (occupancy.IsOccupied(newTileX, newTileY)) {
            m_path.clear();
            return;
        }
        
        const int oldX = GetTileX();
        const int oldY = GetTileY();
        
        m_prevTileX = oldX;
        m_prevTileY = oldY;
        SetTilePosition(newTileX, newTileY);
        m_isDiagonalMove = (newTileX != m_prevTileX && newTileY != m_prevTileY);
        m_isMoving = true;
        
        // Update facing direction
        SetFacing(DirectionUtil::FromDelta(newTileX - oldX, newTileY - oldY));
        
        // Update occupancy
        occupancy.Move(oldX, oldY, newTileX, newTileY);
    }
}


bool Player::TryReplanPath(const Map& map, OccupancyMap& occupancy)
{
    if (!m_hasDestination) return false;
    
    // Already at destination?
    if (GetTileX() == m_destX && GetTileY() == m_destY) {
        m_hasDestination = false;
        return false;
    }
    
    // Find new path (avoiding occupied tiles)
    auto newPath = Pathfinder::FindPath(GetTileX(), GetTileY(), m_destX, m_destY, map, occupancy);
    
    if (newPath.empty()) {
        // No path available, give up on destination
        m_hasDestination = false;
        return false;
    }
    
    // Check if first tile of new path is available
    const auto& firstTarget = newPath[0];
    const int newTileX = static_cast<int>(firstTarget.x);
    const int newTileY = static_cast<int>(firstTarget.y);
    
    if (occupancy.IsOccupied(newTileX, newTileY)) {
        // Still blocked, will try again later
        return false;
    }
    
    // Start following new path
    m_path = std::move(newPath);
    m_pathIndex = 0;
    
    const int oldX = GetTileX();
    const int oldY = GetTileY();
    
    m_prevTileX = oldX;
    m_prevTileY = oldY;
    SetTilePosition(newTileX, newTileY);
    m_isDiagonalMove = (newTileX != m_prevTileX && newTileY != m_prevTileY);
    m_isMoving = true;
    
    // Update facing direction
    SetFacing(DirectionUtil::FromDelta(newTileX - oldX, newTileY - oldY));
    
    // Update occupancy
    occupancy.Move(oldX, oldY, newTileX, newTileY);
    return true;
}

void Player::ClearPath() noexcept
{
    m_path.clear();
    m_pathIndex = 0;
    m_hasDestination = false;
}

bool Player::TryPunch()
{
    // Can only punch if not already punching
    if (!IsPunching()) {
        StartPunch();
        m_punchHitProcessed = false;  // Reset hit processing for new punch
        return true;
    }
    return false;
}

int Player::CalculateDamage(std::mt19937& rng) const
{
    // Base damage with ±10% random variation
    std::uniform_real_distribution<float> variation(0.9f, 1.1f);
    float damage = m_baseAttack * variation(rng);
    
    // 10% critical hit chance for 2x damage
    std::uniform_real_distribution<float> critRoll(0.0f, 1.0f);
    if (critRoll(rng) < m_critChance) {
        damage *= m_critMultiplier;
    }
    
    return static_cast<int>(std::round(damage));
}

Enemy* Player::ProcessPunchHit(std::vector<Enemy>& enemies, std::mt19937& rng)
{
    // Only process hit once per punch, at the peak of the animation (around 50%)
    if (m_punchHitProcessed || !IsPunching()) {
        return nullptr;
    }
    
    // Check if we're at the peak of the punch (40-60% progress)
    const float progress = GetPunchProgress();
    if (progress < 0.4f || progress > 0.6f) {
        return nullptr;
    }
    
    m_punchHitProcessed = true;
    
    // Calculate target tile based on facing direction
    const int targetX = GetTileX() + DirectionUtil::GetDeltaX(GetFacing());
    const int targetY = GetTileY() + DirectionUtil::GetDeltaY(GetFacing());
    
    // Find enemy at target position
    for (auto& enemy : enemies) {
        if (!enemy.IsAlive()) continue;
        
        if (enemy.GetTileX() == targetX && enemy.GetTileY() == targetY) {
            // Hit! Calculate and apply damage
            const int damage = CalculateDamage(rng);
            enemy.TakeDamage(damage);
            return &enemy;
        }
    }
    
    return nullptr;  // No hit
}

void Player::Update(float deltaTime, const Map& map, OccupancyMap& occupancy)
{
    // Update punch animation
    UpdatePunch(deltaTime);
    
    // If we have a destination but no current path, try to re-plan
    if (m_hasDestination && m_path.empty() && !m_isMoving) {
        TryReplanPath(map, occupancy);
    }
    
    const float targetX = static_cast<float>(GetTileX());
    const float targetY = static_cast<float>(GetTileY());
    const float dx = targetX - GetRenderX();
    const float dy = targetY - GetRenderY();
    const float distSq = dx * dx + dy * dy;
    
    constexpr float kArrivalThresholdSq = 0.01f * 0.01f;
    
    // Check if we've arrived at current target
    if (distSq <= kArrivalThresholdSq) {
        SetRenderPosition(targetX, targetY);
        
        // Check if we've reached final destination
        if (m_hasDestination && GetTileX() == m_destX && GetTileY() == m_destY) {
            m_path.clear();
            m_isMoving = false;
            m_isDiagonalMove = false;
            m_hasDestination = false;
            return;
        }
        
        // If following a path, advance to next waypoint
        if (!m_path.empty() && m_pathIndex < m_path.size()) {
            ++m_pathIndex;
            
            if (m_pathIndex < m_path.size()) {
                // Set up next waypoint
                const auto& nextTarget = m_path[m_pathIndex];
                const int newTileX = static_cast<int>(nextTarget.x);
                const int newTileY = static_cast<int>(nextTarget.y);
                
                // Check if next tile is occupied
                if (occupancy.IsOccupied(newTileX, newTileY)) {
                    // Path blocked, try to re-plan
                    m_path.clear();
                    m_isMoving = false;
                    m_isDiagonalMove = false;
                    
                    // Try immediate re-plan
                    TryReplanPath(map, occupancy);
                    return;
                }
                
                const int oldX = GetTileX();
                const int oldY = GetTileY();
                
                m_prevTileX = oldX;
                m_prevTileY = oldY;
                SetTilePosition(newTileX, newTileY);
                m_isDiagonalMove = (newTileX != m_prevTileX && newTileY != m_prevTileY);
                
                // Update facing direction
                SetFacing(DirectionUtil::FromDelta(newTileX - oldX, newTileY - oldY));
                
                // Update occupancy
                occupancy.Move(oldX, oldY, newTileX, newTileY);
            } else {
                // Path complete
                m_path.clear();
                m_isMoving = false;
                m_isDiagonalMove = false;
                
                // If we still have a destination, we haven't reached it
                if (m_hasDestination) {
                    TryReplanPath(map, occupancy);
                }
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
        SetRenderPosition(targetX, targetY);
    } else {
        const float invDist = 1.0f / dist;
        SetRenderPosition(
            GetRenderX() + dx * invDist * moveAmount,
            GetRenderY() + dy * invDist * moveAmount
        );
    }
}
