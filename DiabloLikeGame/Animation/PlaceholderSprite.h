#pragma once

#include "raylib.h"
#include <cmath>

// Creates a placeholder sprite sheet texture at runtime for testing
// This allows testing animations without external art assets
class PlaceholderSprite {
public:
    // Generate a placeholder sprite sheet texture
    // Returns a valid Texture2D that must be unloaded when done
    static Texture2D GeneratePlayerSheet()
    {
        TraceLog(LOG_INFO, "Generating Player Sheet from PlaceholderSprite.h (New Colors)");
        constexpr int kFrameWidth = 64;
        constexpr int kFrameHeight = 64;
        constexpr int kDirections = 8;
        
        constexpr int kIdleFrames = 4;
        constexpr int kWalkFrames = 8;
        constexpr int kAttackFrames = 6;
        constexpr int kHitFrames = 3;
        constexpr int kDieFrames = 6;
        constexpr int kTotalRows = kIdleFrames + kWalkFrames + kAttackFrames + kHitFrames + kDieFrames;
        
        const int width = kFrameWidth * kDirections;
        const int height = kFrameHeight * kTotalRows;
        
        // Create image
        Image image = GenImageColor(width, height, BLANK);
        
        // Direction angles (S, SW, W, NW, N, NE, E, SE) in radians
        // Adjusted for isometric view where N is Top-Right (45 degrees)
        const float angles[] = {
            3.92699f,      // S (225бу - Down-Left)
            4.71239f,      // SW (270бу - Left)
            5.49779f,      // W (315бу - Up-Left)
            0.0f,          // NW (0бу - Up)
            0.78540f,      // N (45бу - Up-Right)
            1.57080f,      // NE (90бу - Right)
            2.35619f,      // E (135бу - Down-Right)
            3.14159f       // SE (180бу - Down)
        };
        
        // State colors
        const Color stateColors[] = {
            {194, 178, 128, 255},  // Idle - Ecru/Beige (╦╪╥┬)
            {180, 160, 110, 255},  // Walk - Darker Beige
            {205, 92, 92, 255},    // Attack - Indian Red (Earthy)
            {200, 50, 50, 255},    // Hit - Bright Red (Clearly visible)
            {128, 128, 128, 255},  // Die - Gray
        };
        
        int currentRow = 0;
        
        // Helper to draw a filled circle on image
        auto drawCircle = [&image, width](int cx, int cy, int radius, Color color) {
            for (int y = -radius; y <= radius; ++y) {
                for (int x = -radius; x <= radius; ++x) {
                    if (x * x + y * y <= radius * radius) {
                        int px = cx + x;
                        int py = cy + y;
                        if (px >= 0 && px < width && py >= 0) {
                            ImageDrawPixel(&image, px, py, color);
                        }
                    }
                }
            }
        };
        
        // Helper to draw a line on image
        auto drawLine = [&image, width, height](int x0, int y0, int x1, int y1, Color color) {
            int dx = std::abs(x1 - x0);
            int dy = std::abs(y1 - y0);
            int sx = x0 < x1 ? 1 : -1;
            int sy = y0 < y1 ? 1 : -1;
            int err = dx - dy;
            
            while (true) {
                if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height) {
                    ImageDrawPixel(&image, x0, y0, color);
                }
                if (x0 == x1 && y0 == y1) break;
                int e2 = 2 * err;
                if (e2 > -dy) { err -= dy; x0 += sx; }
                if (e2 < dx) { err += dx; y0 += sy; }
            }
        };
        
        // Generate frames for each state
        auto generateFrames = [&](int startRow, int frameCount, Color baseColor, bool isAttack = false, bool isDie = false, bool isHit = false) {
            for (int frame = 0; frame < frameCount; ++frame) {
                for (int dir = 0; dir < kDirections; ++dir) {
                    const int x = dir * kFrameWidth + kFrameWidth / 2;
                    const int y = (startRow + frame) * kFrameHeight + kFrameHeight / 2;
                    
                    float progress = static_cast<float>(frame) / frameCount;
                    
                    // Calculate radius based on state
                    int radius = 18;
                    int xOffset = 0;
                    int yOffset = 0;
                    
                    if (isAttack) {
                        radius = 15 + static_cast<int>(progress * 12);
                    } else if (isDie) {
                        radius = 18 - static_cast<int>(progress * 8);
                        yOffset = static_cast<int>(progress * 8);
                    } else if (isHit) {
                        // Recoil effect (shake/shrink)
                        radius = 18 - static_cast<int>(std::sin(progress * 3.14f) * 4.0f);
                        // Shake horizontally relative to direction? simple jitter for now
                        xOffset = (frame % 2 == 0) ? -2 : 2;
                    } else {
                        float pulse = 0.9f + 0.1f * std::sin(progress * 6.28f);
                        radius = static_cast<int>(18 * pulse);
                    }
                    
                    // Adjust color alpha for die animation
                    Color color = baseColor;
                    if (isDie) {
                        color.a = static_cast<unsigned char>(255 * (1.0f - progress * 0.6f));
                    }
                    
                    // Draw body
                    drawCircle(x + xOffset, y + yOffset, radius, color);
                    
                    // Draw direction indicator (white line)
                    if (!isDie) {  // Don't draw direction when dying
                        float angle = angles[dir];
                        int lineLen = isAttack ? (12 + static_cast<int>(progress * 6)) : 12;
                        int endX = x + xOffset + static_cast<int>(std::sin(angle) * lineLen);
                        int endY = y + yOffset - static_cast<int>(std::cos(angle) * lineLen);
                        drawLine(x + xOffset, y + yOffset, endX, endY, WHITE);
                    }
                }
            }
        };
        
        // Idle
        generateFrames(currentRow, kIdleFrames, stateColors[0]);
        currentRow += kIdleFrames;
        
        // Walk
        generateFrames(currentRow, kWalkFrames, stateColors[1]);
        currentRow += kWalkFrames;
        
        // Attack
        generateFrames(currentRow, kAttackFrames, stateColors[2], true, false);
        currentRow += kAttackFrames;
        
        // Hit
        generateFrames(currentRow, kHitFrames, stateColors[3], false, false, true);
        currentRow += kHitFrames;
        
        // Die
        generateFrames(currentRow, kDieFrames, stateColors[4], false, true);
        
        // Convert to texture
        Texture2D texture = LoadTextureFromImage(image);
        UnloadImage(image);
        
        return texture;
    }
    
    // Generate enemy placeholder (different color scheme)
    static Texture2D GenerateEnemySheet(Color baseColor)
    {
        // Similar to player but with different base color
        // For now, use the same structure
        constexpr int kFrameWidth = 64;
        constexpr int kFrameHeight = 64;
        constexpr int kDirections = 8;
        constexpr int kTotalRows = 4 + 8 + 6 + 3 + 6; // Same as player
        
        const int width = kFrameWidth * kDirections;
        const int height = kFrameHeight * kTotalRows;
        
        Image image = GenImageColor(width, height, BLANK);
        
        // Simple enemy representation - just colored squares
        for (int row = 0; row < kTotalRows; ++row) {
            for (int dir = 0; dir < kDirections; ++dir) {
                int x = dir * kFrameWidth;
                int y = row * kFrameHeight;
                
                // Draw a simple square
                ImageDrawRectangle(&image, x + 16, y + 16, 32, 32, baseColor);
            }
        }
        
        Texture2D texture = LoadTextureFromImage(image);
        UnloadImage(image);
        
        return texture;
    }
};
