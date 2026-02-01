#pragma once

#include "raylib.h"

// Isometric tile dimensions (compile-time constants)
namespace TileConstants {
    inline constexpr int TILE_WIDTH = 64;
    inline constexpr int TILE_HEIGHT = 32;
    inline constexpr int TILE_DEPTH = 20;
}

// Tile colors - can be loaded from config/tiles.ini
class TileColors {
public:
    static bool Load(const char* filename = "config/tiles.ini");
    
    static Color FloorFill() { return s_floorFill; }
    static Color FloorOutline() { return s_floorOutline; }
    static Color WallTop() { return s_wallTop; }
    static Color WallLeft() { return s_wallLeft; }
    static Color WallRight() { return s_wallRight; }
    static Color WaterFill() { return s_waterFill; }
    static Color WaterOutline() { return s_waterOutline; }
    static Color Shadow() { return s_shadow; }
    static Color PathLine() { return s_pathLine; }

private:
    inline static Color s_floorFill = {60, 60, 65, 255};
    inline static Color s_floorOutline = {40, 40, 45, 255};
    inline static Color s_wallTop = {100, 100, 110, 255};
    inline static Color s_wallLeft = {70, 70, 80, 255};
    inline static Color s_wallRight = {85, 85, 95, 255};
    inline static Color s_waterFill = {50, 100, 150, 200};
    inline static Color s_waterOutline = {30, 80, 130, 200};
    inline static Color s_shadow = {0, 0, 0, 80};
    inline static Color s_pathLine = {144, 238, 144, 200};
};
