#pragma once

#include "raylib.h"
#include "Map.h"
#include <vector>

// Forward declarations
class Player;
class Enemy;
class Entity;
class GameCamera;

class IsometricRenderer {
public:
    IsometricRenderer() = default;
    ~IsometricRenderer() = default;

    // Set camera reference (required before drawing)
    void SetCamera(GameCamera* camera) noexcept { m_camera = camera; }
    [[nodiscard]] GameCamera* GetCamera() const noexcept { return m_camera; }

    // Coordinate conversion
    [[nodiscard]] Vector2 TileToScreen(float tileX, float tileY) const noexcept;
    [[nodiscard]] Vector2 ScreenToTile(int screenX, int screenY) const noexcept;

    // Drawing with proper depth sorting
    void DrawScene(const Map& map, const Player& player, Color playerColor,
                   const std::vector<Enemy>& enemies, Color enemyColor) const;
    
    // Individual drawing (for simple cases without depth sorting)
    void DrawTile(int x, int y, Color color, Color outlineColor) const;
    void DrawBlock(int x, int y, Color topColor, Color leftColor, Color rightColor) const;
    void DrawPlayer(const Player& player, Color color) const;
    void DrawMap(const Map& map) const;
    
    // Draw pathfinding visualization (dashed line on ground)
    void DrawPath(const Player& player, Color color) const;

private:
    void DrawEntityAt(float tileX, float tileY, Color color) const;
    void DrawHealthBar(const Entity& entity, bool isPlayer) const;
    [[nodiscard]] bool IsTileVisible(int x, int y) const noexcept;
    void GetVisibleTileRange(const Map& map, int& startX, int& startY, int& endX, int& endY) const noexcept;

    GameCamera* m_camera{nullptr};
};
