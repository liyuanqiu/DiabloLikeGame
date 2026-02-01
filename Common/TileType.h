#pragma once

#include <cstdint>

// Tile types - shared across all projects
enum class TileType : uint8_t {
    Empty = 0,
    Floor = 1,
    Wall = 2,
    Water = 3
};
