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

void IsometricRenderer::DrawEntityAt(float tileX, float tileY, Color color, bool isPlayer, 
                                      Direction facing, float punchProgress) const
{
    const auto pos = TileToScreen(tileX, tileY);
    
    // Entity dimensions
    constexpr float bodyHeight = 16.0f;
    constexpr float bodyWidth = 14.0f;
    constexpr float headRadius = 6.0f;
    
    // Calculate center position (standing on tile)
    const float baseY = pos.y + TILE_HEIGHT / 2.0f;  // Bottom of entity (on ground)
    const float centerX = pos.x;
    
    // Draw direction arrow first (under shadow)
    DrawDirectionArrow(centerX, baseY, facing, color);
    
    // Draw shadow
    DrawEllipse(static_cast<int>(centerX), static_cast<int>(baseY), 
                10.0f, 5.0f, TileColors::Shadow());
    
    // Calculate body outline color
    const Color outlineColor = { 
        static_cast<unsigned char>(color.r * 0.5f),
        static_cast<unsigned char>(color.g * 0.5f),
        static_cast<unsigned char>(color.b * 0.5f),
        255 
    };
    
    // Calculate lighter color for head/face and hands
    const Color lightColor = {
        static_cast<unsigned char>(std::min(255, color.r + 40)),
        static_cast<unsigned char>(std::min(255, color.g + 40)),
        static_cast<unsigned char>(std::min(255, color.b + 40)),
        255
    };
    
    // Skin color for hands
    const Color skinColor = {255, 220, 185, 255};
    const Color skinOutline = {180, 150, 120, 255};
    
    // Body - trapezoid shape (wider at bottom like a robe/cloak)
    const float bodyTop = baseY - bodyHeight - headRadius * 2.0f;
    const float bodyBottom = baseY - 2.0f;
    const float topWidth = bodyWidth * 0.6f;
    const float bottomWidth = bodyWidth;
    
    const Vector2 bodyTL = {centerX - topWidth / 2.0f, bodyTop};
    const Vector2 bodyTR = {centerX + topWidth / 2.0f, bodyTop};
    const Vector2 bodyBL = {centerX - bottomWidth / 2.0f, bodyBottom};
    const Vector2 bodyBR = {centerX + bottomWidth / 2.0f, bodyBottom};
    
    // Draw body (two triangles for trapezoid)
    DrawTriangle(bodyTL, bodyBL, bodyBR, color);
    DrawTriangle(bodyTL, bodyBR, bodyTR, color);
    
    // Body outline
    DrawLineV(bodyTL, bodyTR, outlineColor);
    DrawLineV(bodyTR, bodyBR, outlineColor);
    DrawLineV(bodyBR, bodyBL, outlineColor);
    DrawLineV(bodyBL, bodyTL, outlineColor);
    
    // Draw arms
    DrawArms(centerX, bodyTop, bodyBottom, skinColor, skinOutline, facing, punchProgress);
    
    // Head - circle
    const float headCenterY = bodyTop - headRadius - 1.0f;
    DrawCircle(static_cast<int>(centerX), static_cast<int>(headCenterY), 
               headRadius, lightColor);
    DrawCircleLines(static_cast<int>(centerX), static_cast<int>(headCenterY), 
                    headRadius, outlineColor);
    
    if (isPlayer) {
        // Player: friendly eyes (small dots) and smile
        constexpr float eyeOffset = 2.5f;
        constexpr float eyeRadius = 1.2f;
        const float eyeY = headCenterY - 1.0f;
        
        // Eyes
        DrawCircle(static_cast<int>(centerX - eyeOffset), static_cast<int>(eyeY), 
                   eyeRadius, outlineColor);
        DrawCircle(static_cast<int>(centerX + eyeOffset), static_cast<int>(eyeY), 
                   eyeRadius, outlineColor);
        
        // Small smile (arc)
        const float smileY = headCenterY + 2.0f;
        DrawLine(static_cast<int>(centerX - 2), static_cast<int>(smileY),
                 static_cast<int>(centerX), static_cast<int>(smileY + 1), outlineColor);
        DrawLine(static_cast<int>(centerX), static_cast<int>(smileY + 1),
                 static_cast<int>(centerX + 2), static_cast<int>(smileY), outlineColor);
    } else {
        // Enemy: angry eyes (slanted) and frown
        constexpr float eyeOffset = 2.5f;
        const float eyeY = headCenterY - 1.0f;
        
        // Angry eyes (slanted lines)
        DrawLine(static_cast<int>(centerX - eyeOffset - 1), static_cast<int>(eyeY - 1),
                 static_cast<int>(centerX - eyeOffset + 1), static_cast<int>(eyeY + 1), outlineColor);
        DrawLine(static_cast<int>(centerX + eyeOffset - 1), static_cast<int>(eyeY + 1),
                 static_cast<int>(centerX + eyeOffset + 1), static_cast<int>(eyeY - 1), outlineColor);
        
        // Frown
        const float frownY = headCenterY + 2.5f;
        DrawLine(static_cast<int>(centerX - 2), static_cast<int>(frownY + 1),
                 static_cast<int>(centerX), static_cast<int>(frownY), outlineColor);
        DrawLine(static_cast<int>(centerX), static_cast<int>(frownY),
                 static_cast<int>(centerX + 2), static_cast<int>(frownY + 1), outlineColor);
    }
}

void IsometricRenderer::DrawArms(float centerX, float bodyTop, float bodyBottom, Color color,
                                  Color outlineColor, Direction facing, float punchProgress) const
{
    // Arm dimensions
    constexpr float armLength = 8.0f;
    constexpr float handRadius = 3.0f;
    constexpr float punchExtend = 12.0f;  // Extra extension when punching
    
    // Arm attachment point (middle of body sides)
    const float armY = (bodyTop + bodyBottom) / 2.0f - 2.0f;
    const float armOffsetX = 8.0f;  // Distance from center to arm
    
    // Get direction vector for punching
    const int dx = DirectionUtil::GetDeltaX(facing);
    const int dy = DirectionUtil::GetDeltaY(facing);
    
    // Convert to isometric screen direction
    const float isoX = static_cast<float>(dx - dy) * 0.5f;
    const float isoY = static_cast<float>(dx + dy) * 0.25f;
    
    // Normalize
    const float len = std::sqrt(isoX * isoX + isoY * isoY);
    const float dirX = len > 0.001f ? isoX / len : 0.0f;
    const float dirY = len > 0.001f ? isoY / len : 0.0f;
    
    // Punch animation: 0->0.5 extend, 0.5->1.0 retract
    float punchAmount = 0.0f;
    if (punchProgress > 0.0f) {
        if (punchProgress < 0.5f) {
            punchAmount = punchProgress * 2.0f;  // 0 to 1
        } else {
            punchAmount = (1.0f - punchProgress) * 2.0f;  // 1 to 0
        }
    }
    
    // Draw left arm (always at rest position)
    const float leftArmX = centerX - armOffsetX;
    const float leftHandX = leftArmX - armLength * 0.5f;
    const float leftHandY = armY + 2.0f;
    
    // Left arm line
    DrawLineEx({leftArmX, armY}, {leftHandX, leftHandY}, 2.0f, outlineColor);
    // Left hand (fist)
    DrawCircle(static_cast<int>(leftHandX), static_cast<int>(leftHandY), handRadius, color);
    DrawCircleLines(static_cast<int>(leftHandX), static_cast<int>(leftHandY), handRadius, outlineColor);
    
    // Draw right arm (punching arm)
    const float rightArmX = centerX + armOffsetX;
    float rightHandX = rightArmX + armLength * 0.5f;
    float rightHandY = armY + 2.0f;
    
    // Apply punch extension in facing direction
    if (punchAmount > 0.0f) {
        rightHandX += dirX * punchExtend * punchAmount;
        rightHandY += dirY * punchExtend * punchAmount;
    }
    
    // Right arm line
    DrawLineEx({rightArmX, armY}, {rightHandX, rightHandY}, 2.0f, outlineColor);
    // Right hand (fist)
    DrawCircle(static_cast<int>(rightHandX), static_cast<int>(rightHandY), handRadius, color);
    DrawCircleLines(static_cast<int>(rightHandX), static_cast<int>(rightHandY), handRadius, outlineColor);
}

void IsometricRenderer::DrawDirectionArrow(float screenX, float screenY, Direction facing, Color color) const
{
    // Convert grid direction to isometric screen direction
    // In isometric view:
    // Grid +X (East) -> Screen right-up
    // Grid +Y (South) -> Screen right-down
    // Grid -X (West) -> Screen left-down
    // Grid -Y (North) -> Screen left-up
    
    const int dx = DirectionUtil::GetDeltaX(facing);
    const int dy = DirectionUtil::GetDeltaY(facing);
    
    // Convert to isometric screen offset
    // Isometric: screenX = (tileX - tileY) * halfWidth, screenY = (tileX + tileY) * halfHeight
    const float isoX = static_cast<float>(dx - dy) * (TILE_WIDTH / 4.0f);
    const float isoY = static_cast<float>(dx + dy) * (TILE_HEIGHT / 4.0f);
    
    // Arrow properties
    constexpr float arrowLength = 18.0f;
    constexpr float arrowHeadSize = 6.0f;
    
    // Normalize and scale
    const float len = std::sqrt(isoX * isoX + isoY * isoY);
    if (len < 0.001f) return;
    
    const float dirX = isoX / len;
    const float dirY = isoY / len;
    
    // Arrow start (at entity feet) and end
    const Vector2 start = {screenX, screenY};
    const Vector2 end = {screenX + dirX * arrowLength, screenY + dirY * arrowLength};
    
    // Arrow head points
    const float perpX = -dirY;
    const float perpY = dirX;
    const Vector2 head1 = {
        end.x - dirX * arrowHeadSize + perpX * arrowHeadSize * 0.5f,
        end.y - dirY * arrowHeadSize + perpY * arrowHeadSize * 0.5f
    };
    const Vector2 head2 = {
        end.x - dirX * arrowHeadSize - perpX * arrowHeadSize * 0.5f,
        end.y - dirY * arrowHeadSize - perpY * arrowHeadSize * 0.5f
    };
    
    // Arrow color (bright yellow for visibility)
    const Color arrowColor = {255, 220, 50, 255};
    const Color arrowOutline = {180, 150, 30, 255};
    
    // Draw arrow shaft (outline first, then fill)
    DrawLineEx(start, end, 4.0f, arrowOutline);
    DrawLineEx(start, end, 2.0f, arrowColor);
    
    // Draw arrow head using lines to ensure visibility
    // Outline
    DrawLineEx(end, head1, 4.0f, arrowOutline);
    DrawLineEx(end, head2, 4.0f, arrowOutline);
    
    // Fill
    DrawLineEx(end, head1, 2.0f, arrowColor);
    DrawLineEx(end, head2, 2.0f, arrowColor);
}

void IsometricRenderer::DrawPlayer(const Player& player, Color color) const
{
    // If player has a sprite, render sprite; otherwise use colored entity
    if (player.HasSprite()) {
        DrawPlayerSprite(player);
    } else {
        DrawEntityAt(player.GetRenderX(), player.GetRenderY(), color, true, 
                     player.GetFacing(), player.GetPunchProgress());
    }
}

void IsometricRenderer::DrawPlayerSprite(const Player& player) const
{
    const auto& animator = player.GetAnimator();
    const Texture2D& texture = animator.GetTexture();
    Rectangle srcRect = animator.GetSourceRect();
    
    // Get screen position
    const auto pos = TileToScreen(player.GetRenderX(), player.GetRenderY());
    
    // Center sprite on tile position
    // Sprite is drawn with bottom-center at tile position
    const float destX = pos.x - srcRect.width / 2.0f;
    const float destY = pos.y - srcRect.height + TILE_HEIGHT / 2.0f + 8.0f;  // Adjust for standing on tile
    
    Rectangle destRect = {destX, destY, srcRect.width, srcRect.height};
    
    // Draw the sprite
    DrawTexturePro(texture, srcRect, destRect, {0, 0}, 0.0f, WHITE);
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
    constexpr float barOffsetY = -32.0f;  // Above the entity head (adjusted for new character height)
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
        Direction facing;
        float punchProgress;
        const Entity* entity;  // For health bar drawing
    };
    std::vector<EntityInfo> entities;
    entities.reserve(enemies.size() + 1);
    
    // Add player
    entities.push_back({playerDepth, playerX, playerY, playerColor, true, 
                        player.GetFacing(), player.GetPunchProgress(), &player});
    
    // Add visible enemies (use their individual color from config)
    for (const auto& enemy : enemies) {
        if (!enemy.IsAlive()) continue;
        if (!IsTileVisible(enemy.GetTileX(), enemy.GetTileY())) continue;
        
        const float ex = enemy.GetRenderX();
        const float ey = enemy.GetRenderY();
        entities.push_back({ex + ey, ex, ey, enemy.GetColor(), false, 
                           enemy.GetFacing(), enemy.GetPunchProgress(), &enemy});
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
                bool drawn = false;
                if (e.isPlayer && e.entity) {
                    const Player* player = static_cast<const Player*>(e.entity);
                    if (player->HasSprite()) {
                        // Draw shadow
                        DrawEllipse(static_cast<int>(e.renderX), static_cast<int>(e.renderY + TILE_HEIGHT / 2.0f), 
                                    10.0f, 5.0f, TileColors::Shadow());
                        DrawPlayerSprite(*player);
                        drawn = true;
                    }
                }
                
                if (!drawn) {
                    DrawEntityAt(e.renderX, e.renderY, e.color, e.isPlayer, e.facing, e.punchProgress);
                }
                
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
        
        bool drawn = false;
        if (e.isPlayer && e.entity) {
            const Player* player = static_cast<const Player*>(e.entity);
            if (player->HasSprite()) {
                // Draw shadow
                DrawEllipse(static_cast<int>(e.renderX), static_cast<int>(e.renderY + TILE_HEIGHT / 2.0f), 
                            10.0f, 5.0f, TileColors::Shadow());
                DrawPlayerSprite(*player);
                drawn = true;
            }
        }
        
        if (!drawn) {
            DrawEntityAt(e.renderX, e.renderY, e.color, e.isPlayer, e.facing, e.punchProgress);
        }

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
