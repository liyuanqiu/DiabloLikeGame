#include "IsometricRenderer.h"
#include "Player.h"
#include "Camera/Camera.h"
#include "Core/TileConstants.h"
#include <cassert>
#include <cmath>

using namespace TileConstants;

// Pre-defined tile colors (avoid construction every frame)
namespace TileColors {
    static constexpr Color FloorFill    = {60, 60, 65, 255};
    static constexpr Color FloorOutline = {40, 40, 45, 255};
    static constexpr Color WallTop      = {100, 100, 110, 255};
    static constexpr Color WallLeft     = {70, 70, 80, 255};
    static constexpr Color WallRight    = {85, 85, 95, 255};
    static constexpr Color WaterFill    = {50, 100, 150, 200};
    static constexpr Color WaterOutline = {30, 80, 130, 200};
    static constexpr Color Shadow       = {0, 0, 0, 80};
    static constexpr Color PathLine     = {144, 238, 144, 200};  // Light green for path
}

Vector2 IsometricRenderer::TileToScreen(float tileX, float tileY) const noexcept
{
    assert(m_camera && "Camera must be set before rendering");
    return m_camera->TileToScreen(tileX, tileY);
}

Vector2 IsometricRenderer::ScreenToTile(int screenX, int screenY) const noexcept
{
    assert(m_camera && "Camera must be set before rendering");
    return m_camera->ScreenToTile(screenX, screenY);
}

void IsometricRenderer::DrawPlayerAt(float tileX, float tileY, Color color) const
{
    const auto pos = TileToScreen(tileX, tileY);
    
    constexpr float playerHeight = 24.0f;
    constexpr float playerWidth = 20.0f;
    constexpr float halfHeight = playerHeight / 2.0f;
    constexpr float halfWidth = playerWidth / 2.0f;
    
    const float centerY = pos.y + TILE_HEIGHT / 2.0f - halfHeight;
    const Vector2 top    = {pos.x, centerY - halfHeight};
    const Vector2 bottom = {pos.x, centerY + halfHeight};
    const Vector2 left   = {pos.x - halfWidth, centerY};
    const Vector2 right  = {pos.x + halfWidth, centerY};
    
    // Draw shadow first
    DrawEllipse(static_cast<int>(pos.x), static_cast<int>(pos.y + TILE_HEIGHT / 2.0f), 
                10.0f, 5.0f, TileColors::Shadow);
    
    // Draw player body
    DrawTriangle(top, left, bottom, color);
    DrawTriangle(top, bottom, right, color);
    
    const Color outlineColor = { 
        static_cast<unsigned char>(color.r * 0.6f),
        static_cast<unsigned char>(color.g * 0.6f),
        static_cast<unsigned char>(color.b * 0.6f),
        255 
    };
    DrawLineV(top, left, outlineColor);
    DrawLineV(left, bottom, outlineColor);
    DrawLineV(bottom, right, outlineColor);
    DrawLineV(right, top, outlineColor);
}

void IsometricRenderer::DrawPlayer(const Player& player, Color color) const
{
    DrawPlayerAt(player.GetRenderX(), player.GetRenderY(), color);
}

bool IsometricRenderer::IsTileVisible(int x, int y) const noexcept
{
    return m_camera->IsTileVisible(x, y);
}

void IsometricRenderer::GetVisibleTileRange(const Map& map, int& startX, int& startY, int& endX, int& endY) const noexcept
{
    m_camera->GetVisibleTileRange(map.GetWidth(), map.GetHeight(), startX, startY, endX, endY);
}

void IsometricRenderer::DrawTile(int x, int y, Color color, Color outlineColor) const
{
    const auto pos = TileToScreen(static_cast<float>(x), static_cast<float>(y));
    
    const Vector2 top = { pos.x, pos.y };
    const Vector2 right = { pos.x + TILE_WIDTH / 2.0f, pos.y + TILE_HEIGHT / 2.0f };
    const Vector2 bottom = { pos.x, pos.y + TILE_HEIGHT };
    const Vector2 left = { pos.x - TILE_WIDTH / 2.0f, pos.y + TILE_HEIGHT / 2.0f };
    
    DrawTriangle(top, left, bottom, color);
    DrawTriangle(top, bottom, right, color);
    
    DrawLineV(top, right, outlineColor);
    DrawLineV(right, bottom, outlineColor);
    DrawLineV(bottom, left, outlineColor);
    DrawLineV(left, top, outlineColor);
}

void IsometricRenderer::DrawBlock(int x, int y, Color topColor, Color leftColor, Color rightColor) const
{
    const auto pos = TileToScreen(static_cast<float>(x), static_cast<float>(y));
    
    const Vector2 topTop = { pos.x, pos.y - TILE_DEPTH };
    const Vector2 topRight = { pos.x + TILE_WIDTH / 2.0f, pos.y + TILE_HEIGHT / 2.0f - TILE_DEPTH };
    const Vector2 topBottom = { pos.x, pos.y + TILE_HEIGHT - TILE_DEPTH };
    const Vector2 topLeft = { pos.x - TILE_WIDTH / 2.0f, pos.y + TILE_HEIGHT / 2.0f - TILE_DEPTH };
    
    const Vector2 bottomRight = { pos.x + TILE_WIDTH / 2.0f, pos.y + TILE_HEIGHT / 2.0f };
    const Vector2 bottomBottom = { pos.x, pos.y + TILE_HEIGHT };
    const Vector2 bottomLeft = { pos.x - TILE_WIDTH / 2.0f, pos.y + TILE_HEIGHT / 2.0f };
    
    DrawTriangle(topLeft, bottomLeft, bottomBottom, leftColor);
    DrawTriangle(topLeft, bottomBottom, topBottom, leftColor);
    
    DrawTriangle(topRight, topBottom, bottomBottom, rightColor);
    DrawTriangle(topRight, bottomBottom, bottomRight, rightColor);
    
    DrawTriangle(topTop, topLeft, topBottom, topColor);
    DrawTriangle(topTop, topBottom, topRight, topColor);
    
    DrawLineV(topTop, topRight, BLACK);
    DrawLineV(topRight, topBottom, BLACK);
    DrawLineV(topBottom, topLeft, BLACK);
    DrawLineV(topLeft, topTop, BLACK);
    DrawLineV(topLeft, bottomLeft, BLACK);
    DrawLineV(bottomLeft, bottomBottom, BLACK);
    DrawLineV(bottomBottom, bottomRight, BLACK);
    DrawLineV(bottomRight, topRight, BLACK);
    DrawLineV(topBottom, bottomBottom, BLACK);
}

void IsometricRenderer::DrawMap(const Map& map) const
{
    int startX, startY, endX, endY;
    GetVisibleTileRange(map, startX, startY, endX, endY);
    
    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {
            if (!IsTileVisible(x, y)) continue;
            
            switch (map.GetTileUnchecked(x, y)) {
                case TileType::Empty:
                    break;
                case TileType::Floor:
                    DrawTile(x, y, TileColors::FloorFill, TileColors::FloorOutline);
                    break;
                case TileType::Wall:
                    DrawBlock(x, y, TileColors::WallTop, TileColors::WallLeft, TileColors::WallRight);
                    break;
                case TileType::Water:
                    DrawTile(x, y, TileColors::WaterFill, TileColors::WaterOutline);
                    break;
            }
        }
    }
}

void IsometricRenderer::DrawScene(const Map& map, const Player& player, Color playerColor) const
{
    int startX, startY, endX, endY;
    GetVisibleTileRange(map, startX, startY, endX, endY);
    
    // Get player tile position for occlusion check
    const float playerX = player.GetRenderX();
    const float playerY = player.GetRenderY();
    const int playerTileX = static_cast<int>(std::floor(playerX));
    const int playerTileY = static_cast<int>(std::floor(playerY));
    
    // Occlusion rule: wall occludes player if wall is in S, SE, or E direction
    // S:  x == playerX, y > playerY
    // SE: x > playerX,  y > playerY  
    // E:  x > playerX,  y == playerY
    // Combined: (x >= playerX AND y >= playerY) AND (x > playerX OR y > playerY)
    auto wallOccludesPlayer = [playerTileX, playerTileY](int x, int y) {
        return (x >= playerTileX && y >= playerTileY) && (x > playerTileX || y > playerTileY);
    };
    
    // Pass 1: Draw all floor and water tiles
    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {
            if (!IsTileVisible(x, y)) continue;
            
            const TileType tile = map.GetTileUnchecked(x, y);
            if (tile == TileType::Floor) {
                DrawTile(x, y, TileColors::FloorFill, TileColors::FloorOutline);
            } else if (tile == TileType::Water) {
                DrawTile(x, y, TileColors::WaterFill, TileColors::WaterOutline);
            }
        }
    }
    
    // Pass 1.5: Draw pathfinding visualization on ground
    DrawPath(player, TileColors::PathLine);
    
    // Pass 2: Draw walls that do NOT occlude player (N, NE, W, NW, SW)
    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {
            if (!IsTileVisible(x, y)) continue;
            if (map.GetTileUnchecked(x, y) != TileType::Wall) continue;
            
            if (!wallOccludesPlayer(x, y)) {
                DrawBlock(x, y, TileColors::WallTop, TileColors::WallLeft, TileColors::WallRight);
            }
        }
    }
    
    // Pass 3: Draw player
    DrawPlayerAt(playerX, playerY, playerColor);
    
    // Pass 4: Draw walls that DO occlude player (S, SE, E)
    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {
            if (!IsTileVisible(x, y)) continue;
            if (map.GetTileUnchecked(x, y) != TileType::Wall) continue;
            
            if (wallOccludesPlayer(x, y)) {
                DrawBlock(x, y, TileColors::WallTop, TileColors::WallLeft, TileColors::WallRight);
            }
        }
    }
}

void IsometricRenderer::DrawPath(const Player& player, Color color) const
{
    const auto& path = player.GetPath();
    const size_t pathIndex = player.GetPathIndex();
    
    // No path or path completed
    if (path.empty() || pathIndex >= path.size()) {
        return;
    }
    
    // Start from player's current render position
    Vector2 prevScreen = TileToScreen(player.GetRenderX(), player.GetRenderY());
    // Offset to tile center (bottom of diamond)
    prevScreen.y += TILE_HEIGHT / 2.0f;
    
    // Draw dashed lines along the remaining path
    constexpr float dashLength = 8.0f;
    constexpr float gapLength = 6.0f;
    
    for (size_t i = pathIndex; i < path.size(); ++i) {
        Vector2 currScreen = TileToScreen(path[i].x, path[i].y);
        currScreen.y += TILE_HEIGHT / 2.0f;
        
        // Calculate line direction and length
        const float dx = currScreen.x - prevScreen.x;
        const float dy = currScreen.y - prevScreen.y;
        const float length = std::sqrt(dx * dx + dy * dy);
        
        if (length > 0.0f) {
            const float dirX = dx / length;
            const float dirY = dy / length;
            
            // Draw dashed line segment
            float drawn = 0.0f;
            bool drawing = true;
            
            while (drawn < length) {
                const float segmentLength = drawing ? dashLength : gapLength;
                const float remaining = length - drawn;
                const float actualLength = (segmentLength < remaining) ? segmentLength : remaining;
                
                if (drawing) {
                    const Vector2 start = {
                        prevScreen.x + dirX * drawn,
                        prevScreen.y + dirY * drawn
                    };
                    const Vector2 end = {
                        prevScreen.x + dirX * (drawn + actualLength),
                        prevScreen.y + dirY * (drawn + actualLength)
                    };
                    DrawLineEx(start, end, 2.0f, color);
                }
                
                drawn += actualLength;
                drawing = !drawing;
            }
        }
        
        // Draw a small dot at each waypoint
        DrawCircleV(currScreen, 3.0f, color);
        
        prevScreen = currScreen;
    }
}
