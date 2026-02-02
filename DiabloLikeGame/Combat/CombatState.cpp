#include "CombatState.h"
#include "../Entity.h"
#include "../Enemy.h"

void EnemyCombatState::CleanupThreatList()
{
    // Collect dead entities first (can't erase while iterating SmallSet)
    Entity* deadEntities[8];
    size_t deadCount = 0;
    
    for (auto* entity : threatList) {
        if (entity == nullptr || !entity->IsAlive()) {
            if (deadCount < 8) {
                deadEntities[deadCount++] = entity;
            }
        }
    }
    
    // Remove dead entities
    for (size_t i = 0; i < deadCount; ++i) {
        threatList.erase(deadEntities[i]);
    }
    
    // Clear current target if dead
    if (currentTarget && !currentTarget->IsAlive()) {
        currentTarget = nullptr;
        
        // Try to find another target
        if (!threatList.empty()) {
            currentTarget = *threatList.begin();
        }
    }
}

bool EnemyCombatState::HasThreatInVision(int currentX, int currentY) const
{
    for (const auto* entity : threatList) {
        if (entity && entity->IsAlive()) {
            if (IsInVisionRange(currentX, currentY, 
                               entity->GetTileX(), entity->GetTileY())) {
                return true;
            }
        }
    }
    return false;
}

void PlayerCombatState::CleanupDeadEnemies()
{
    // Collect dead enemies first
    Enemy* deadEnemies[16];
    size_t deadCount = 0;
    
    for (auto* enemy : engagedEnemies) {
        if (enemy == nullptr || !enemy->IsAlive()) {
            if (deadCount < 16) {
                deadEnemies[deadCount++] = enemy;
            }
        }
    }
    
    // Remove dead enemies
    for (size_t i = 0; i < deadCount; ++i) {
        engagedEnemies.erase(deadEnemies[i]);
    }
    
    if (engagedEnemies.empty()) {
        inCombat = false;
    }
}
