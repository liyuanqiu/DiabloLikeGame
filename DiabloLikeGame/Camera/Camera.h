#pragma once

#include "raylib.h"

// GameCamera class - manages view position and coordinate conversion
// (Named GameCamera to avoid conflict with raylib's Camera)
class GameCamera {
public:
    GameCamera() = default;
    ~GameCamera() = default;

    // Initialize camera
    void Init(int screenWidth, int screenHeight);
    
    // Position control
    void SetPosition(float x, float y) noexcept;
    void Move(float deltaX, float deltaY) noexcept;
    void CenterOn(float worldX, float worldY) noexcept;
    
    // Getters
    [[nodiscard]] float GetX() const noexcept { return m_x; }
    [[nodiscard]] float GetY() const noexcept { return m_y; }
    [[nodiscard]] int GetScreenWidth() const noexcept { return m_screenWidth; }
    [[nodiscard]] int GetScreenHeight() const noexcept { return m_screenHeight; }
    
    // Coordinate conversion (tile <-> screen)
    [[nodiscard]] Vector2 TileToScreen(float tileX, float tileY) const noexcept;
    [[nodiscard]] Vector2 ScreenToTile(int screenX, int screenY) const noexcept;
    
    // Visibility check
    [[nodiscard]] bool IsTileVisible(int tileX, int tileY) const noexcept;
    
    // Get visible tile range for culling
    void GetVisibleTileRange(int mapWidth, int mapHeight, 
                             int& startX, int& startY, 
                             int& endX, int& endY) const noexcept;

private:
    float m_x{};  // Camera position (screen offset)
    float m_y{};
    int m_screenWidth{1920};
    int m_screenHeight{1080};
};
