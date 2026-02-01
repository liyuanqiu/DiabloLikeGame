#pragma once

#include "raylib.h"
#include "Core/Direction.h"

// Base class for all game entities (player, enemies, NPCs, etc.)
class Entity {
public:
    Entity() = default;
    Entity(int tileX, int tileY);
    virtual ~Entity() = default;
    
    // Non-copyable but movable
    Entity(const Entity&) = default;
    Entity& operator=(const Entity&) = default;
    Entity(Entity&&) = default;
    Entity& operator=(Entity&&) = default;
    
    // Position getters
    [[nodiscard]] int GetTileX() const noexcept { return m_tileX; }
    [[nodiscard]] int GetTileY() const noexcept { return m_tileY; }
    [[nodiscard]] float GetRenderX() const noexcept { return m_renderX; }
    [[nodiscard]] float GetRenderY() const noexcept { return m_renderY; }
    
    // Depth for isometric sorting (x + y)
    [[nodiscard]] float GetDepth() const noexcept { return m_renderX + m_renderY; }
    
    // Direction/Facing
    [[nodiscard]] Direction GetFacing() const noexcept { return m_facing; }
    void SetFacing(Direction dir) noexcept { m_facing = dir; }
    void FaceToward(int targetX, int targetY) noexcept;
    
    // Attack/Punch state
    [[nodiscard]] bool IsPunching() const noexcept { return m_isPunching; }
    [[nodiscard]] float GetPunchProgress() const noexcept { return m_punchProgress; }
    [[nodiscard]] bool IsPunchHitProcessed() const noexcept { return m_punchHitProcessed; }
    void SetPunchHitProcessed(bool processed) noexcept { m_punchHitProcessed = processed; }
    void SetPunchDuration(float duration) noexcept { m_punchDuration = duration; }
    void StartPunch() noexcept;
    void UpdatePunch(float deltaTime) noexcept;
    
    // Alive state
    [[nodiscard]] bool IsAlive() const noexcept { return m_isAlive; }
    void Kill() noexcept { m_isAlive = false; }
    
    // Health system
    [[nodiscard]] int GetHealth() const noexcept { return m_health; }
    [[nodiscard]] int GetMaxHealth() const noexcept { return m_maxHealth; }
    [[nodiscard]] float GetHealthPercent() const noexcept { 
        return m_maxHealth > 0 ? static_cast<float>(m_health) / static_cast<float>(m_maxHealth) : 0.0f; 
    }
    [[nodiscard]] bool HasBeenDamaged() const noexcept { return m_hasBeenDamaged; }
    
    void SetHealth(int health, int maxHealth) noexcept;
    void TakeDamage(int amount) noexcept;
    void Heal(int amount) noexcept;
    
    // Position setters
    void SetTilePosition(int tileX, int tileY) noexcept;
    void SetRenderPosition(float renderX, float renderY) noexcept;

protected:
    // Tile position (grid coordinates)
    int m_tileX{};
    int m_tileY{};
    
    // Render position (for smooth animation)
    float m_renderX{};
    float m_renderY{};
    
    // Direction facing
    Direction m_facing{Direction::South};  // Default facing camera
    
    // Punch animation state
    bool m_isPunching{false};
    float m_punchProgress{0.0f};  // 0.0 = start, 1.0 = fully extended, back to 0.0 = retracted
    float m_punchDuration{0.25f}; // Configurable punch animation time
    bool m_punchHitProcessed{false};  // Track if current punch has dealt damage
    
    // Health
    int m_health{100};
    int m_maxHealth{100};
    bool m_hasBeenDamaged{false};
    
    // State
    bool m_isAlive{true};
};
