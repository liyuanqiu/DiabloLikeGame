#include "Enemy.h"
#include "Player.h"
#include "Common/Map.h"
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
    m_combatState.spawnX = tileX;
    m_combatState.spawnY = tileY;
    
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
    m_combatState.spawnX = tileX;
    m_combatState.spawnY = tileY;
    
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

void Enemy::TakeDamage(int amount, Entity* attacker) noexcept
{
    Entity::TakeDamage(amount);
    
    if (amount > 0 && IsAlive()) {
        // Record damage and attacker
        m_combatState.OnDamageReceived(attacker);
        
        // Enter combat based on aggression type
        if (!m_combatState.inCombat) {
            m_combatState.EnterCombat(attacker);
            
            switch (m_aggressionType) {
                case AggressionType::Passive:
                    m_combatState.behavior = CombatBehavior::Fleeing;
                    break;
                case AggressionType::Defensive:
                    m_combatState.behavior = CombatBehavior::Chasing;
                    m_isAggressive = true;
                    break;
                case AggressionType::Aggressive:
                    m_combatState.behavior = CombatBehavior::Chasing;
                    break;
            }
        }
    }
}

void Enemy::ExitCombat() noexcept
{
    m_combatState.Reset();
    m_isAggressive = (m_aggressionType == AggressionType::Aggressive);
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
                m_combatState.OnAttackSuccess();  // Reset combat timer
                
                // Player enters combat when attacked
                player->GetCombatState().AddEnemy(this);
            }
        }
    }
    
    // Update combat timers
    if (m_combatState.inCombat) {
        m_combatState.combatTimer += deltaTime;
        m_combatState.lastDamageReceivedTime += deltaTime;
        m_combatState.CleanupThreatList();
    }
    
    // Handle movement animation
    if (m_isMoving) {
        const float targetX = static_cast<float>(GetTileX());
        const float targetY = static_cast<float>(GetTileY());
        const float dx = targetX - GetRenderX();
        const float dy = targetY - GetRenderY();
        const float distSq = dx * dx + dy * dy;
        
        constexpr float kArrivalThresholdSq = 0.01f * 0.01f;
        
        if (distSq <= kArrivalThresholdSq) {
            SetRenderPosition(targetX, targetY);
            m_isMoving = false;
            m_isDiagonalMove = false;
            
            // Pause after arriving (use configured pause time for wandering)
            if (m_combatState.behavior == CombatBehavior::Wandering) {
                std::uniform_real_distribution<float> pauseDist(m_pauseTimeMin, m_pauseTimeMax);
                m_pauseTimer = pauseDist(rng);
            } else {
                // Short pause for combat movement
                std::uniform_real_distribution<float> pauseDist(0.1f, 0.3f);
                m_pauseTimer = pauseDist(rng);
            }
        } else {
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
        return;  // Don't process behavior while moving
    }
    
    // Aggressive enemies always check for player in vision, even while paused
    if (m_aggressionType == AggressionType::Aggressive && 
        !m_combatState.inCombat && 
        player && player->IsAlive()) {
        if (EnemyCombatState::IsInVisionRange(GetTileX(), GetTileY(),
                                             player->GetTileX(), player->GetTileY())) {
            // Spotted player! Enter combat immediately
            m_combatState.EnterCombat(player);
            m_combatState.behavior = CombatBehavior::Chasing;
            player->GetCombatState().AddEnemy(this);
            m_pauseTimer = 0.0f;  // Cancel any pause
        }
    }
    
    // Wait for pause timer
    if (m_pauseTimer > 0.0f) {
        m_pauseTimer -= deltaTime;
        return;
    }
    
    // Behavior based on state
    switch (m_combatState.behavior) {
        case CombatBehavior::Returning:
            UpdateReturningBehavior(deltaTime, map, occupancy, rng);
            break;
        case CombatBehavior::Wandering:
            UpdateWanderingBehavior(deltaTime, map, occupancy, rng, player);
            break;
        case CombatBehavior::Fleeing:
        case CombatBehavior::Chasing:
        case CombatBehavior::Attacking:
            // Route to aggression-specific behavior
            switch (m_aggressionType) {
                case AggressionType::Passive:
                    UpdatePassiveBehavior(deltaTime, map, occupancy, rng, player);
                    break;
                case AggressionType::Defensive:
                    UpdateDefensiveBehavior(deltaTime, map, occupancy, rng, player);
                    break;
                case AggressionType::Aggressive:
                    UpdateAggressiveBehavior(deltaTime, map, occupancy, rng, player);
                    break;
            }
            break;
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

// Try to move toward a specific target
bool Enemy::TryMoveToward(int targetX, int targetY, const Map& map, OccupancyMap& occupancy)
{
    const int currentX = GetTileX();
    const int currentY = GetTileY();
    
    // Calculate direction to target
    const int dirX = (targetX > currentX) ? 1 : (targetX < currentX) ? -1 : 0;
    const int dirY = (targetY > currentY) ? 1 : (targetY < currentY) ? -1 : 0;
    
    // Try directions in order of preference
    // 1. Direct path, 2. X only, 3. Y only, 4. Opposite diagonal
    int tries[][2] = {
        {dirX, dirY},
        {dirX, 0},
        {0, dirY},
        {dirX, -dirY},
        {-dirX, dirY}
    };
    
    for (const auto& t : tries) {
        if (t[0] == 0 && t[1] == 0) continue;
        
        const int newX = currentX + t[0];
        const int newY = currentY + t[1];
        
        if (!Pathfinder::IsTileWalkable(map, newX, newY)) continue;
        if (occupancy.IsOccupied(newX, newY)) continue;
        
        const bool isDiagonal = (t[0] != 0 && t[1] != 0);
        if (isDiagonal) {
            if (!Pathfinder::IsTileWalkable(map, currentX + t[0], currentY) ||
                !Pathfinder::IsTileWalkable(map, currentX, currentY + t[1])) {
                continue;
            }
        }
        
        // Valid move
        m_prevTileX = currentX;
        m_prevTileY = currentY;
        SetTilePosition(newX, newY);
        m_isMoving = true;
        m_isDiagonalMove = isDiagonal;
        SetFacing(DirectionUtil::FromDelta(t[0], t[1]));
        occupancy.Move(currentX, currentY, newX, newY);
        return true;
    }
    
    return false;
}

// Try to move away from a threat (for Passive enemies)
bool Enemy::TryMoveAwayFrom(int threatX, int threatY, const Map& map, 
                            OccupancyMap& occupancy, std::mt19937& rng)
{
    const int currentX = GetTileX();
    const int currentY = GetTileY();
    
    // Calculate direction away from threat
    const int awayX = (currentX > threatX) ? 1 : (currentX < threatX) ? -1 : 0;
    const int awayY = (currentY > threatY) ? 1 : (currentY < threatY) ? -1 : 0;
    
    // Build list of preferred escape directions (away from threat)
    std::vector<std::pair<int, int>> directions;
    
    // Direct away
    if (awayX != 0 || awayY != 0) {
        directions.push_back({awayX, awayY});
    }
    // Perpendicular options (randomized)
    if (awayX != 0) {
        directions.push_back({awayX, 1});
        directions.push_back({awayX, -1});
    }
    if (awayY != 0) {
        directions.push_back({1, awayY});
        directions.push_back({-1, awayY});
    }
    // Side moves
    directions.push_back({0, 1});
    directions.push_back({0, -1});
    directions.push_back({1, 0});
    directions.push_back({-1, 0});
    
    // Shuffle a bit for variety
    if (directions.size() > 2) {
        std::shuffle(directions.begin() + 1, directions.end(), rng);
    }
    
    for (const auto& [dx, dy] : directions) {
        if (dx == 0 && dy == 0) continue;
        
        const int newX = currentX + dx;
        const int newY = currentY + dy;
        
        if (!Pathfinder::IsTileWalkable(map, newX, newY)) continue;
        if (occupancy.IsOccupied(newX, newY)) continue;
        
        const bool isDiagonal = (dx != 0 && dy != 0);
        if (isDiagonal) {
            if (!Pathfinder::IsTileWalkable(map, currentX + dx, currentY) ||
                !Pathfinder::IsTileWalkable(map, currentX, currentY + dy)) {
                continue;
            }
        }
        
        m_prevTileX = currentX;
        m_prevTileY = currentY;
        SetTilePosition(newX, newY);
        m_isMoving = true;
        m_isDiagonalMove = isDiagonal;
        SetFacing(DirectionUtil::FromDelta(dx, dy));
        occupancy.Move(currentX, currentY, newX, newY);
        return true;
    }
    
    return false;
}

bool Enemy::TryMoveTowardSpawn(const Map& map, OccupancyMap& occupancy)
{
    return TryMoveToward(m_combatState.spawnX, m_combatState.spawnY, map, occupancy);
}

// === BEHAVIOR IMPLEMENTATIONS ===

void Enemy::UpdatePassiveBehavior(float deltaTime, const Map& map, OccupancyMap& occupancy,
                                  std::mt19937& rng, Player* player)
{
    // Passive: flee when in combat, never attack
    if (!m_combatState.inCombat) {
        m_combatState.behavior = CombatBehavior::Wandering;
        return;
    }
    
    // Check exit conditions:
    // 1. 20 seconds since last damage AND no threats in vision
    const bool timeoutReached = m_combatState.lastDamageReceivedTime >= EnemyCombatState::kCombatTimeout;
    const bool noThreatsNearby = !m_combatState.HasThreatInVision(GetTileX(), GetTileY());
    
    if (timeoutReached && noThreatsNearby) {
        // Start returning to spawn
        m_combatState.behavior = CombatBehavior::Returning;
        return;
    }
    
    // Flee from the threat
    if (m_combatState.currentTarget && m_combatState.currentTarget->IsAlive()) {
        TryMoveAwayFrom(m_combatState.currentTarget->GetTileX(),
                       m_combatState.currentTarget->GetTileY(),
                       map, occupancy, rng);
    }
}

void Enemy::UpdateDefensiveBehavior(float deltaTime, const Map& map, OccupancyMap& occupancy,
                                    std::mt19937& rng, Player* player)
{
    // Defensive: chase and attack when provoked
    if (!m_combatState.inCombat) {
        m_combatState.behavior = CombatBehavior::Wandering;
        m_isAggressive = false;
        return;
    }
    
    // Check exit conditions
    if (m_combatState.ShouldGiveUp() || 
        m_combatState.IsBeyondLeash(GetTileX(), GetTileY())) {
        m_combatState.behavior = CombatBehavior::Returning;
        return;
    }
    
    Entity* target = m_combatState.currentTarget;
    if (!target || !target->IsAlive()) {
        // Lost target
        m_combatState.behavior = CombatBehavior::Returning;
        return;
    }
    
    // Check if in attack range
    const int dx = target->GetTileX() - GetTileX();
    const int dy = target->GetTileY() - GetTileY();
    const bool inRange = (std::abs(dx) <= 1 && std::abs(dy) <= 1 && (dx != 0 || dy != 0));
    
    if (inRange) {
        // Attack!
        m_combatState.behavior = CombatBehavior::Attacking;
        FaceToward(target->GetTileX(), target->GetTileY());
        
        if (!IsPunching() && m_attackCooldown <= 0.0f) {
            StartPunch();
            m_attackCooldown = m_attackCooldownTime;
        }
    } else {
        // Chase
        m_combatState.behavior = CombatBehavior::Chasing;
        TryMoveToward(target->GetTileX(), target->GetTileY(), map, occupancy);
    }
}

void Enemy::UpdateAggressiveBehavior(float deltaTime, const Map& map, OccupancyMap& occupancy,
                                     std::mt19937& rng, Player* player)
{
    // Aggressive: actively hunt the player
    
    // Check if should spot player (even if not in combat)
    if (!m_combatState.inCombat && player && player->IsAlive()) {
        if (EnemyCombatState::IsInVisionRange(GetTileX(), GetTileY(),
                                             player->GetTileX(), player->GetTileY())) {
            // Spotted player! Enter combat
            m_combatState.EnterCombat(player);
            m_combatState.behavior = CombatBehavior::Chasing;
            
            // Notify player
            player->GetCombatState().AddEnemy(this);
        }
    }
    
    if (!m_combatState.inCombat) {
        m_combatState.behavior = CombatBehavior::Wandering;
        return;
    }
    
    // Check exit conditions
    if (m_combatState.ShouldGiveUp() || 
        m_combatState.IsBeyondLeash(GetTileX(), GetTileY())) {
        m_combatState.behavior = CombatBehavior::Returning;
        return;
    }
    
    Entity* target = m_combatState.currentTarget;
    if (!target || !target->IsAlive()) {
        m_combatState.behavior = CombatBehavior::Returning;
        return;
    }
    
    // Check attack range
    const int dx = target->GetTileX() - GetTileX();
    const int dy = target->GetTileY() - GetTileY();
    const bool inRange = (std::abs(dx) <= 1 && std::abs(dy) <= 1 && (dx != 0 || dy != 0));
    
    if (inRange) {
        m_combatState.behavior = CombatBehavior::Attacking;
        FaceToward(target->GetTileX(), target->GetTileY());
        
        if (!IsPunching() && m_attackCooldown <= 0.0f) {
            StartPunch();
            m_attackCooldown = m_attackCooldownTime;
        }
    } else {
        m_combatState.behavior = CombatBehavior::Chasing;
        TryMoveToward(target->GetTileX(), target->GetTileY(), map, occupancy);
    }
}

void Enemy::UpdateReturningBehavior(float deltaTime, const Map& map, OccupancyMap& occupancy,
                                    std::mt19937& rng)
{
    // Check if arrived at spawn
    if (GetTileX() == m_combatState.spawnX && GetTileY() == m_combatState.spawnY) {
        // Fully reset combat state
        ExitCombat();
        m_combatState.behavior = CombatBehavior::Wandering;
        
        // Random pause before wandering
        std::uniform_real_distribution<float> pauseDist(m_pauseTimeMin, m_pauseTimeMax);
        m_pauseTimer = pauseDist(rng);
        return;
    }
    
    // Move toward spawn
    if (!TryMoveTowardSpawn(map, occupancy)) {
        // Blocked, wait a bit
        m_pauseTimer = 0.5f;
    }
}

void Enemy::UpdateWanderingBehavior(float deltaTime, const Map& map, OccupancyMap& occupancy,
                                    std::mt19937& rng, Player* player)
{
    // Aggressive enemies check for player in vision
    if (m_aggressionType == AggressionType::Aggressive && player && player->IsAlive()) {
        if (EnemyCombatState::IsInVisionRange(GetTileX(), GetTileY(),
                                             player->GetTileX(), player->GetTileY())) {
            m_combatState.EnterCombat(player);
            m_combatState.behavior = CombatBehavior::Chasing;
            player->GetCombatState().AddEnemy(this);
            return;
        }
    }
    
    // Normal wandering - try to move, pause is set when movement completes
    if (!TryMoveOneStep(map, occupancy, rng)) {
        // Couldn't move, wait a bit before trying again
        std::uniform_real_distribution<float> pauseDist(0.5f, 1.5f);
        m_pauseTimer = pauseDist(rng);
    }
    // Note: pause after successful move is set in Update() when m_isMoving becomes false
}
