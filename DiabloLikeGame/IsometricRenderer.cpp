#include "IsometricRenderer.h"
#include "Player.h"
#include "Enemy.h"
#include "Entity.h"
#include "Camera/Camera.h"
#include "Core/TileConstants.h"
#include <cassert>
#include <cmath>
#include <algorithm>
#include <vector>

using namespace TileConstants;

// Now using TileColors class from TileConstants.h

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

void IsometricRenderer::DrawEntityAt(float tileX, float tileY, Color color) const
{
    const auto pos = TileToScreen(tileX, tileY);
    
    constexpr float entityHeight = 24.0f;
    constexpr float entityWidth = 20.0f;
    constexpr float halfHeight = entityHeight / 2.0f;
    constexpr float halfWidth = entityWidth / 2.0f;
    
    const float centerY = pos.y + TILE_HEIGHT / 2.0f - halfHeight;
    const Vector2 top    = {pos.x, centerY - halfHeight};
    const Vector2 bottom = {pos.x, centerY + halfHeight};
    const Vector2 left   = {pos.x - halfWidth, centerY};
    const Vector2 right  = {pos.x + halfWidth, centerY};
    
    // Draw shadow first
    DrawEllipse(static_cast<int>(pos.x), static_cast<int>(pos.y + TILE_HEIGHT / 2.0f), 
                10.0f, 5.0f, TileColors::Shadow());
    
    // Draw entity body
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
    DrawEntityAt(player.GetRenderX(), player.GetRenderY(), color);
}

void IsometricRenderer::DrawHealthBar(const Entity& entity, bool isPlayer) const
{
    // Show health bar if:
    // 1. Entity has taken damage, OR
    // 2. Entity's max health is not 100 (random health), OR  
    // 3. Entity's current health is less than max health
    const bool shouldShow = entity.HasBeenDamaged() || 
                           entity.GetMaxHealth() != 100 ||
                           entity.GetHealth() < entity.GetMaxHealth();
    if (!shouldShow) return;
    
    const auto pos = TileToScreen(entity.GetRenderX(), entity.GetRenderY());
    
    // Health bar dimensions
    constexpr float barWidth = 24.0f;
    constexpr float barHeight = 4.0f;
    constexpr float barOffsetY = -20.0f;  // Above the entity
    constexpr float baselineHealth = 100.0f;  // Reference health for bar display
    
    const float barX = pos.x - barWidth / 2.0f;
    const float barY = pos.y + barOffsetY;
    
    // Draw background (dark gray)
    DrawRectangle(static_cast<int>(barX), static_cast<int>(barY), 
                  static_cast<int>(barWidth), static_cast<int>(barHeight), 
                  Color{40, 40, 40, 200});
    
    // Calculate health percentage relative to baseline (100)
    // This way, an entity with 30/30 HP shows 30% bar, not 100%
    const float healthPercent = static_cast<float>(entity.GetHealth()) / baselineHealth;
    const float fillWidth = barWidth * std::min(healthPercent, 1.0f);
    
    // Determine bar color based on current health relative to baseline
    Color barColor;
    if (isPlayer) {
        // Player: Green (100-60%) -> Yellow (60-20%) -> Red (20-0%)
        if (healthPercent > 0.6f) {
            barColor = Color{0, 200, 0, 255};      // Green
        } else if (healthPercent > 0.2f) {
            barColor = Color{255, 200, 0, 255};    // Yellow
        } else {
            barColor = Color{200, 0, 0, 255};      // Red
        }
    } else {
        // Enemy: Always red
        barColor = Color{200, 0, 0, 255};
    }
    
    // Draw health fill
    if (fillWidth > 0) {
        DrawRectangle(static_cast<int>(barX), static_cast<int>(barY), 
                      static_cast<int>(fillWidth), static_cast<int>(barHeight), 
                      barColor);
    }
    
    // Draw border
    DrawRectangleLines(static_cast<int>(barX), static_cast<int>(barY), 
                       static_cast<int>(barWidth), static_cast<int>(barHeight), 
                       Color{0, 0, 0, 255});
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
                    DrawTile(x, y, TileColors::FloorFill(), TileColors::FloorOutline());
                    break;
                case TileType::Wall:
                    DrawBlock(x, y, TileColors::WallTop(), TileColors::WallLeft(), TileColors::WallRight());
                    break;
                case TileType::Water:
                    DrawTile(x, y, TileColors::WaterFill(), TileColors::WaterOutline());
                    break;
            }
        }
    }
}

void IsometricRenderer::DrawScene(const Map& map, const Player& player, Color playerColor,
                                   const std::vector<Enemy>& enemies, Color enemyColor) const
{
    int startX, startY, endX, endY;
    GetVisibleTileRange(map, startX, startY, endX, endY);
    
    // Cache colors to avoid method calls in tight loops
    const Color floorFill = TileColors::FloorFill();
    const Color floorOutline = TileColors::FloorOutline();
    const Color wallTop = TileColors::WallTop();
    const Color wallLeft = TileColors::WallLeft();
    const Color wallRight = TileColors::WallRight();
    const Color waterFill = TileColors::WaterFill();
    const Color waterOutline = TileColors::WaterOutline();
    const Color pathLine = TileColors::PathLine();
    
    // Pass 1: Draw all floor and water tiles (ground layer)
    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {
            if (!IsTileVisible(x, y)) continue;
            
            const TileType tile = map.GetTileUnchecked(x, y);
            if (tile == TileType::Floor) {
                DrawTile(x, y, floorFill, floorOutline);
            } else if (tile == TileType::Water) {
                DrawTile(x, y, waterFill, waterOutline);
            }
        }
    }
    
    // Pass 1.5: Draw pathfinding visualization on ground
    DrawPath(player, pathLine);
    
    // Pass 2: Draw walls and entities with proper depth sorting
    // In isometric view (camera at NW looking SE), depth = x + y
    // Lower depth values are further from camera and drawn first
    // Higher depth values are closer to camera and can occlude things behind them
    
    // Get player depth for sorting
    const float playerX = player.GetRenderX();
    const float playerY = player.GetRenderY();
    const float playerDepth = playerX + playerY;
    
    // Build a list of visible entities with their depths
    struct EntityInfo {
        float depth;
        float renderX;
        float renderY;
        Color color;
        bool isPlayer;
        const Entity* entity;  // For health bar drawing
    };
    std::vector<EntityInfo> entities;
    entities.reserve(enemies.size() + 1);
    
    // Add player
    entities.push_back({playerDepth, playerX, playerY, playerColor, true, &player});
    
    // Add visible enemies
    for (const auto& enemy : enemies) {
        if (!enemy.IsAlive()) continue;
        if (!IsTileVisible(enemy.GetTileX(), enemy.GetTileY())) continue;
        
        const float ex = enemy.GetRenderX();
        const float ey = enemy.GetRenderY();
        entities.push_back({ex + ey, ex, ey, enemyColor, false, &enemy});
    }
    
    // Sort entities by depth (ascending - further from camera first)
    std::sort(entities.begin(), entities.end(), 
        [](const EntityInfo& a, const EntityInfo& b) { return a.depth < b.depth; });
    
    // Draw walls and entities in depth order
    // For each row of depth (y coordinate in tile space), draw walls then entities at that depth
    size_t entityIdx = 0;
    
    // Iterate through tile rows in depth order (x + y = constant defines a row)
    // Min depth is startX + startY, max depth is endX + endY
    for (int depth = startX + startY; depth <= endX + endY; ++depth) {
        // Draw all walls at this depth
        for (int x = std::max(startX, depth - endY); x <= std::min(endX, depth - startY); ++x) {
            const int y = depth - x;
            if (y < startY || y > endY) continue;
            if (!IsTileVisible(x, y)) continue;
            if (map.GetTileUnchecked(x, y) != TileType::Wall) continue;
            
            DrawBlock(x, y, wallTop, wallLeft, wallRight);
        }
        
        // Draw all entities at this depth (entities with depth in [depth, depth+1))
        while (entityIdx < entities.size() && entities[entityIdx].depth < static_cast<float>(depth + 1)) {
            const auto& e = entities[entityIdx];
            if (e.depth >= static_cast<float>(depth)) {
                DrawEntityAt(e.renderX, e.renderY, e.color);
                // Draw health bar above entity
                if (e.entity) {
                    DrawHealthBar(*e.entity, e.isPlayer);
                }
            }
            ++entityIdx;
        }
    }
    
    // Draw any remaining entities (in case of rounding issues)
    for (; entityIdx < entities.size(); ++entityIdx) {
        const auto& e = entities[entityIdx];
        DrawEntityAt(e.renderX, e.renderY, e.color);
        if (e.entity) {
            DrawHealthBar(*e.entity, e.isPlayer);
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
