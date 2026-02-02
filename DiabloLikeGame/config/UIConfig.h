#pragma once

#include "raylib.h"

// UI configuration constants
// All UI-related visual parameters are centralized here
namespace UIConfig {

    // === Input Mode Dropdown ===
    namespace Dropdown {
        inline constexpr int kWidth = 180;
        inline constexpr int kHeight = 30;
        inline constexpr int kItemHeight = 28;
        inline constexpr int kMarginRight = 200;           // From screen right edge
        inline constexpr int kMarginTop = 10;
        
        inline constexpr Color kHeaderNormal = {50, 50, 70, 255};
        inline constexpr Color kHeaderHover = {70, 70, 90, 255};
        inline constexpr Color kItemNormal = {40, 40, 60, 255};
        inline constexpr Color kItemHover = {60, 60, 100, 255};
        inline constexpr Color kItemSelected = {80, 80, 120, 255};
    }

    // === Controller Input Thresholds ===
    namespace Controller {
        inline constexpr float kCardinalThreshold = 0.7f;  // Threshold for cardinal direction
        inline constexpr float kDiagonalThreshold = 0.4f;  // Threshold for diagonal detection
        inline constexpr float kAimThreshold = 0.5f;       // Threshold for aim stick
        inline constexpr float kDirectionRatio = 1.5f;     // Ratio for determining primary axis
    }

    // === Status Indicator ===
    namespace StatusIndicator {
        inline constexpr float kRadius = 5.0f;
        inline constexpr int kOffsetFromRight = 15;
    }
}
