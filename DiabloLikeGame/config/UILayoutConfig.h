#pragma once

// UI Layout configuration constants
// All UI positioning and layout parameters are centralized here
namespace UILayoutConfig {

    // === Debug Info Panel (top-left) ===
    namespace DebugInfo {
        inline constexpr int kMarginLeft = 10;
        inline constexpr int kStartY = 10;
        inline constexpr int kTitleFontSize = 20;
        inline constexpr int kInfoFontSize = 16;
        inline constexpr int kSmallFontSize = 14;
        inline constexpr int kTinyFontSize = 12;
        inline constexpr int kLineSpacing = 20;      // Between major sections
        inline constexpr int kSubLineSpacing = 16;   // Between sub-items
    }

    // === FPS Display ===
    namespace FPS {
        inline constexpr int kOffsetFromBottom = 25;
    }

    // === Punch Hit Detection Window ===
    namespace Combat {
        inline constexpr float kPunchHitWindowStart = 0.4f;
        inline constexpr float kPunchHitWindowEnd = 0.6f;
    }
}
