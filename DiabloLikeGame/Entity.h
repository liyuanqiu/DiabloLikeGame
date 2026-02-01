#pragma once

#include "raylib.h"

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
    
    // Health
    int m_health{100};
    int m_maxHealth{100};
    bool m_hasBeenDamaged{false};
    
    // State
    bool m_isAlive{true};
};
