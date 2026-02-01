#include "Camera.h"
#include "../Core/TileConstants.h"
#include "../Core/GameConfig.h"
#include <algorithm>
#include <cmath>

using namespace TileConstants;


void GameCamera::Init(int screenWidth, int screenHeight)
{
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    m_x = static_cast<float>(screenWidth / 2);
    m_y = Config::CAMERA_INITIAL_Y_OFFSET;
}

void GameCamera::SetPosition(float x, float y) noexcept
{
    m_x = x;
    m_y = y;
}

void GameCamera::Move(float deltaX, float deltaY) noexcept
{
    m_x += deltaX;
    m_y += deltaY;
}

void GameCamera::CenterOn(float worldX, float worldY) noexcept
{
    // Convert world position to screen offset that centers it
    const auto screenPos = TileToScreen(worldX, worldY);
    m_x += (m_screenWidth / 2.0f) - screenPos.x;
    m_y += (m_screenHeight / 2.0f) - screenPos.y;
}

Vector2 GameCamera::TileToScreen(float tileX, float tileY) const noexcept
{
    return {
        (tileX - tileY) * (TILE_WIDTH / 2.0f) + m_x,
        (tileX + tileY) * (TILE_HEIGHT / 2.0f) + m_y
    };
}

Vector2 GameCamera::ScreenToTile(int screenX, int screenY) const noexcept
{
    const float adjustedX = static_cast<float>(screenX) - m_x;
    const float adjustedY = static_cast<float>(screenY) - m_y;
    
    const float tileX = (adjustedX / (TILE_WIDTH / 2.0f) + adjustedY / (TILE_HEIGHT / 2.0f)) / 2.0f;
    const float tileY = (adjustedY / (TILE_HEIGHT / 2.0f) - adjustedX / (TILE_WIDTH / 2.0f)) / 2.0f;
    
    return { tileX, tileY };
}

bool GameCamera::IsTileVisible(int tileX, int tileY) const noexcept
{
    const auto pos = TileToScreen(static_cast<float>(tileX), static_cast<float>(tileY));
    constexpr int padding = TILE_WIDTH + TILE_DEPTH;
    
    return pos.x >= -padding && 
           pos.x <= m_screenWidth + padding &&
           pos.y >= -padding && 
           pos.y <= m_screenHeight + padding;
}

void GameCamera::GetVisibleTileRange(int mapWidth, int mapHeight,
                                  int& startX, int& startY,
                                  int& endX, int& endY) const noexcept
{
    constexpr int margin = 2;
    
    const auto topLeft = ScreenToTile(0, 0);
    const auto topRight = ScreenToTile(m_screenWidth, 0);
    const auto bottomLeft = ScreenToTile(0, m_screenHeight);
    const auto bottomRight = ScreenToTile(m_screenWidth, m_screenHeight);
    
    const int minX = static_cast<int>(std::min({topLeft.x, topRight.x, bottomLeft.x, bottomRight.x})) - margin;
    const int maxX = static_cast<int>(std::max({topLeft.x, topRight.x, bottomLeft.x, bottomRight.x})) + margin;
    const int minY = static_cast<int>(std::min({topLeft.y, topRight.y, bottomLeft.y, bottomRight.y})) - margin;
    const int maxY = static_cast<int>(std::max({topLeft.y, topRight.y, bottomLeft.y, bottomRight.y})) + margin;
    
    startX = std::clamp(minX, 0, mapWidth - 1);
    endX = std::clamp(maxX, 0, mapWidth - 1);
    startY = std::clamp(minY, 0, mapHeight - 1);
    endY = std::clamp(maxY, 0, mapHeight - 1);
}
