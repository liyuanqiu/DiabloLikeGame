#include "TileConstants.h"
#include "IniParser.h"

bool TileColors::Load(const char* filename) {
    IniParser ini;
    if (!ini.Load(filename)) {
        return false;  // Use defaults if file not found
    }
    
    // Colors
    s_floorFill = ini.GetColor("Colors", "FloorFill", s_floorFill);
    s_floorOutline = ini.GetColor("Colors", "FloorOutline", s_floorOutline);
    s_wallTop = ini.GetColor("Colors", "WallTop", s_wallTop);
    s_wallLeft = ini.GetColor("Colors", "WallLeft", s_wallLeft);
    s_wallRight = ini.GetColor("Colors", "WallRight", s_wallRight);
    s_waterFill = ini.GetColor("Colors", "WaterFill", s_waterFill);
    s_waterOutline = ini.GetColor("Colors", "WaterOutline", s_waterOutline);
    s_shadow = ini.GetColor("Colors", "Shadow", s_shadow);
    s_pathLine = ini.GetColor("Colors", "PathLine", s_pathLine);
    
    return true;
}
