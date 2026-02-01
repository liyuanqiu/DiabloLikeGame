#pragma once

#include <cstdint>

// 8-directional facing for entities in isometric view
// Directions are named based on grid coordinates (not screen)
//
//    NW   N   NE
//      \  |  /
//   W ---+--- E
//      /  |  \
//    SW   S   SE
//
enum class Direction : uint8_t {
    South = 0,      // Down-right on screen (default facing camera)
    SouthWest = 1,  // Down on screen
    West = 2,       // Down-left on screen
    NorthWest = 3,  // Left on screen
    North = 4,      // Up-left on screen (facing away from camera)
    NorthEast = 5,  // Up on screen
    East = 6,       // Up-right on screen
    SouthEast = 7,  // Right on screen
    
    Count = 8
};

// Direction utilities
namespace DirectionUtil {
    // Get direction from movement delta
    // Returns South if dx=0 and dy=0
    [[nodiscard]] constexpr Direction FromDelta(int dx, int dy) noexcept {
        // Direction lookup table based on (dx+1, dy+1) as index
        // dx: -1, 0, +1 -> index 0, 1, 2
        // dy: -1, 0, +1 -> index 0, 1, 2
        constexpr Direction table[3][3] = {
            // dy = -1           dy = 0              dy = +1
            {Direction::NorthWest, Direction::West,  Direction::SouthWest}, // dx = -1
            {Direction::North,     Direction::South, Direction::South},     // dx = 0 (default South)
            {Direction::NorthEast, Direction::East,  Direction::SouthEast}  // dx = +1
        };
        
        const int ix = dx + 1;
        const int iy = dy + 1;
        
        if (ix < 0 || ix > 2 || iy < 0 || iy > 2) {
            return Direction::South;
        }
        
        return table[ix][iy];
    }
    
    // Get opposite direction
    [[nodiscard]] constexpr Direction Opposite(Direction dir) noexcept {
        return static_cast<Direction>((static_cast<uint8_t>(dir) + 4) % 8);
    }
    
    // Get delta X for a direction (-1, 0, or 1)
    [[nodiscard]] constexpr int GetDeltaX(Direction dir) noexcept {
        constexpr int dx[] = {0, -1, -1, -1, 0, 1, 1, 1};
        return dx[static_cast<uint8_t>(dir)];
    }
    
    // Get delta Y for a direction (-1, 0, or 1)
    [[nodiscard]] constexpr int GetDeltaY(Direction dir) noexcept {
        constexpr int dy[] = {1, 1, 0, -1, -1, -1, 0, 1};
        return dy[static_cast<uint8_t>(dir)];
    }
    
    // Check if direction is diagonal
    [[nodiscard]] constexpr bool IsDiagonal(Direction dir) noexcept {
        const auto d = static_cast<uint8_t>(dir);
        return (d % 2) == 1; // Odd directions are diagonal
    }
    
    // Get direction name for debugging
    [[nodiscard]] constexpr const char* ToString(Direction dir) noexcept {
        constexpr const char* names[] = {
            "South", "SouthWest", "West", "NorthWest",
            "North", "NorthEast", "East", "SouthEast"
        };
        const auto d = static_cast<uint8_t>(dir);
        return d < 8 ? names[d] : "Invalid";
    }
}
