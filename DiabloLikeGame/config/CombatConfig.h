#pragma once

// Combat configuration compile-time constants
// NOTE: Many of these values can be overridden per-map via MapConfig
// These serve as fallback defaults when MapConfig isn't available
namespace CombatConfig {

    // === Enemy Combat Timers ===
    // NOTE: kCombatTimeout, kLeashDistance, kVisionRange can be overridden in map config
    namespace Enemy {
        inline constexpr float kCombatTimeout = 20.0f;     // Seconds before giving up chase
        inline constexpr int kLeashDistance = 30;          // Max tiles from spawn point
        inline constexpr int kVisionRange = 8;             // Tiles for aggro detection
        
        // Combat movement pauses (not yet in map config)
        inline constexpr float kCombatPauseMin = 0.1f;
        inline constexpr float kCombatPauseMax = 0.3f;
        
        // Stuck recovery
        inline constexpr float kStuckPauseTime = 0.5f;
    }

    // === Spawning ===
    // NOTE: kSafeRadiusFromPlayer can be overridden in map config
    namespace Spawn {
        inline constexpr int kSafeRadiusFromPlayer = 5;    // Min distance from player spawn
    }
}


