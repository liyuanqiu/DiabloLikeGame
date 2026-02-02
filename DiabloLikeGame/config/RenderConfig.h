#pragma once

#include "raylib.h"

// Render configuration constants
// All visual parameters are centralized here for easy tuning
namespace RenderConfig {

    // === Character Body Dimensions ===
    namespace Character {
        inline constexpr float kBodyHeight = 16.0f;
        inline constexpr float kBodyWidth = 14.0f;
        inline constexpr float kHeadRadius = 6.0f;
        inline constexpr float kTopWidthRatio = 0.6f;      // Body top width = kBodyWidth * this
        
        // Arms
        inline constexpr float kArmLength = 8.0f;
        inline constexpr float kHandRadius = 3.0f;
        inline constexpr float kArmOffsetX = 8.0f;         // Distance from center to arm
        inline constexpr float kPunchExtend = 12.0f;       // Extra extension when punching
        
        // Face features
        inline constexpr float kEyeOffset = 2.5f;
        inline constexpr float kEyeRadius = 1.2f;
        
        // Shadow
        inline constexpr float kShadowRadiusX = 10.0f;
        inline constexpr float kShadowRadiusY = 5.0f;
    }

    // === Direction Arrow ===
    namespace Arrow {
        inline constexpr float kLength = 18.0f;
        inline constexpr float kHeadSize = 6.0f;
        inline constexpr float kShaftWidth = 2.0f;
        inline constexpr float kOutlineWidth = 4.0f;
        
        inline constexpr Color kFillColor = {255, 220, 50, 255};
        inline constexpr Color kOutlineColor = {180, 150, 30, 255};
    }

    // === Health Bar ===
    namespace HealthBar {
        inline constexpr float kWidth = 24.0f;
        inline constexpr float kHeight = 4.0f;
        inline constexpr float kOffsetY = -32.0f;          // Above entity head
        inline constexpr float kBaselineHealth = 100.0f;   // Reference health for display
        
        inline constexpr Color kBackground = {40, 40, 40, 200};
        inline constexpr Color kBorder = {0, 0, 0, 255};
        inline constexpr Color kPlayerHealthy = {0, 200, 0, 255};
        inline constexpr Color kPlayerWarning = {255, 200, 0, 255};
        inline constexpr Color kPlayerCritical = {200, 0, 0, 255};
        inline constexpr Color kEnemy = {200, 0, 0, 255};
        
        inline constexpr float kHealthyThreshold = 0.6f;
        inline constexpr float kWarningThreshold = 0.2f;
    }

    // === Hit Effect ===
    namespace HitEffect {
        inline constexpr int kShakeAmount = 2;
        inline constexpr Color kTintColor = {255, 120, 120, 255};
        inline constexpr Color kSpriteTintColor = {255, 100, 100, 255};
    }

    // === Skin Colors ===
    namespace Skin {
        inline constexpr Color kDefault = {255, 220, 185, 255};
        inline constexpr Color kOutline = {180, 150, 120, 255};
    }

    // === Sprite Rendering ===
    namespace Sprite {
        inline constexpr float kVerticalOffset = 8.0f;     // Adjustment for standing on tile
    }

    // === Scene Colors ===
    namespace Scene {
        inline constexpr Color kBackground = {30, 30, 40, 255};
        inline constexpr Color kPlayerDefault = {194, 178, 128, 255};  // Ecru/Beige
        inline constexpr Color kEnemyDefault = {230, 41, 55, 255};     // Red
    }
}
