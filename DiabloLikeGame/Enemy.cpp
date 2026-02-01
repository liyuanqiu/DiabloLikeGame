#include "Enemy.h"
#include "Player.h"
#include "Map.h"
#include "World/Pathfinder.h"
#include "World/OccupancyMap.h"
#include <cmath>
#include <algorithm>

Enemy::Enemy(int tileX, int tileY, std::mt19937& rng)
    : Entity(tileX, tileY)
    , m_spawnX(tileX)
    , m_spawnY(tileY)
    , m_prevTileX(tileX)
    , m_prevTileY(tileY)
{
    SetHealth(100, 100);
    
    std::uniform_real_distribution<float> pauseDist(m_pauseTimeMin, m_pauseTimeMax);
    m_pauseTimer = pauseDist(rng);
}

Enemy::Enemy(int tileX, int tileY, const EnemyTypeConfig& config, std::mt19937& rng)
    : Entity(tileX, tileY)
    , m_spawnX(tileX)
    , m_spawnY(tileY)
    , m_prevTileX(tileX)
    , m_prevTileY(tileY)
{
    ApplyConfig(config);
    
    std::uniform_real_distribution<float> pauseDist(m_pauseTimeMin, m_pauseTimeMax);
    m_pauseTimer = pauseDist(rng);
}

void Enemy::ApplyConfig(const EnemyTypeConfig& config)
{
    m_typeId = config.id;
    m_displayName = config.displayName;
    
    SetHealth(config.maxHealth, config.maxHealth);
    m_baseAttack = config.baseAttack;
    m_attackVariation = config.attackVariation;
    
    m_moveSpeed = config.moveSpeed;
    m_wanderRadius = config.wanderRadius;
    m_pauseTimeMin = config.pauseTimeMin;
    m_pauseTimeMax = config.pauseTimeMax;
    
    m_aggressionType = config.aggression;
    m_attackCooldownTime = config.attackCooldown;
    
    // Aggressive enemies start aggressive, others need to be provoked
    m_isAggressive = (config.aggression == AggressionType::Aggressive);
    
    m_color = {config.colorR, config.colorG, config.colorB, 255};
}

float Enemy::GetCurrentSpeedMultiplier() const noexcept
{
    // Same as player: diagonal moves take longer
    // sqrt(2) / 2 = 1 / sqrt(2) ¡Ö 0.70710678f
    return m_isDiagonalMove ? 0.70710678f : 1.0f;
}

int Enemy::CalculateDamage(std::mt19937& rng) const
{
    // Base damage with variation
    std::uniform_real_distribution<float> variation(1.0f - m_attackVariation, 1.0f + m_attackVariation);
    const float damage = m_baseAttack * variation(rng);
    return static_cast<int>(std::round(damage));
}

void Enemy::TakeDamage(int amount) noexcept
{
    Entity::TakeDamage(amount);
    
    // Defensive enemies become aggressive when hit
    // Passive enemies never become aggressive
    if (amount > 0 && IsAlive() && m_aggressionType == AggressionType::Defensive) {
        m_isAggressive = true;
    }
}

bool Enemy::TryAttackPlayer(Player* player, std::mt19937& rng)
{
    // Passive enemies never attack
    if (m_aggressionType == AggressionType::Passive) return false;
    
    // Defensive enemies only attack when provoked
    if (m_aggressionType == AggressionType::Defensive && !m_isAggressive) return false;
    
    if (!player || !player->IsAlive()) return false;
    
    // Check if player is adjacent (distance = 1)
    const int dx = player->GetTileX() - GetTileX();
    const int dy = player->GetTileY() - GetTileY();
    const int distSq = dx * dx + dy * dy;
    
    // Adjacent means distance of 1 (orthogonal) or 2 (diagonal, but abs(dx)<=1 && abs(dy)<=1)
    if (std::abs(dx) > 1 || std::abs(dy) > 1 || (dx == 0 && dy == 0)) {
        return false;  // Player not adjacent
    }
    
    // Face toward player
    FaceToward(player->GetTileX(), player->GetTileY());
    
    // Start punch if not already punching and cooldown is ready
    if (!IsPunching() && m_attackCooldown <= 0.0f) {
        StartPunch();  // StartPunch() already resets hit processing
        m_attackCooldown = m_attackCooldownTime;
        return true;
    }
    
    return false;
}

void Enemy::Update(float deltaTime, const Map& map, OccupancyMap& occupancy, 
                   std::mt19937& rng, Player* player)
{
    if (!IsAlive()) return;
    
    // Update attack cooldown
    if (m_attackCooldown > 0.0f) {
        m_attackCooldown -= deltaTime;
    }
    
    // Update punch animation
    UpdatePunch(deltaTime);
    
    // Process punch hit at peak of animation
    if (IsPunching() && !IsPunchHitProcessed() && player && player->IsAlive()) {
        const float progress = GetPunchProgress();
        if (progress >= 0.4f && progress <= 0.6f) {
            SetPunchHitProcessed(true);
            
            // Check if player is still in front of us
            const int targetX = GetTileX() + DirectionUtil::GetDeltaX(GetFacing());
            const int targetY = GetTileY() + DirectionUtil::GetDeltaY(GetFacing());
            
            if (player->GetTileX() == targetX && player->GetTileY() == targetY) {
                const int damage = CalculateDamage(rng);
                player->TakeDamage(damage);
            }
        }
    }
    
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
            
            // Random pause before next move
            std::uniform_real_distribution<float> pauseDist(m_pauseTimeMin, m_pauseTimeMax);
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
        // Not moving - try to attack player if adjacent
        if (player && TryAttackPlayer(player, rng)) {
            // Attacking, don't wander
            return;
        }
        
        // Waiting/paused, try to wander
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
        
        // Update facing direction
        SetFacing(DirectionUtil::FromDelta(dx[dir], dy[dir]));
        
        // Update occupancy
        occupancy.Move(currentX, currentY, newX, newY);
        return true;
    }
    
    return false;  // No valid move found
}
