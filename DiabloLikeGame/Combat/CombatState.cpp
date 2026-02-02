#include "CombatState.h"
#include "../Entity.h"
#include "../Enemy.h"

void EnemyCombatState::CleanupThreatList()
{
    for (auto it = threatList.begin(); it != threatList.end(); ) {
        if (*it == nullptr || !(*it)->IsAlive()) {
            it = threatList.erase(it);
        } else {
            ++it;
        }
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
    for (auto it = engagedEnemies.begin(); it != engagedEnemies.end(); ) {
        if (*it == nullptr || !(*it)->IsAlive()) {
            it = engagedEnemies.erase(it);
        } else {
            ++it;
        }
    }
    
    if (engagedEnemies.empty()) {
        inCombat = false;
    }
}
