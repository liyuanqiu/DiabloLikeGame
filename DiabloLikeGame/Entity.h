#pragma once

#include "raylib.h"
#include "Core/Direction.h"
#include "Net/EntityId.h"

// Base class for all game entities (player, enemies, NPCs, etc.)
// 
// Multiplayer Architecture Note:
// Each entity has a unique EntityId for network synchronization. The ID contains:
// - Entity type (player, enemy, projectile)
// - Owner (which player/server owns this entity)
// - Instance number (unique within owner scope)
//
// For server-authoritative multiplayer:
// - Server assigns IDs and has authority over all game state
// - Clients receive entity updates and apply them to local copies
// - Local player input is sent to server for validation
class Entity {
public:
    Entity() = default;
    Entity(int tileX, int tileY);
    Entity(EntityId id, int tileX, int tileY);
    virtual ~Entity() = default;
    
    // Non-copyable but movable
    Entity(const Entity&) = default;
    Entity& operator=(const Entity&) = default;
    Entity(Entity&&) = default;
    Entity& operator=(Entity&&) = default;
    
    // Network identity
    [[nodiscard]] EntityId GetEntityId() const noexcept { return m_entityId; }
    void SetEntityId(EntityId id) noexcept { m_entityId = id; }
    [[nodiscard]] bool HasValidId() const noexcept { return m_entityId.IsValid(); }
    
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
    
    // Hit/Damage reaction state
    [[nodiscard]] bool IsHit() const noexcept { return m_isHit; }
    void UpdateHit(float deltaTime) noexcept;
    
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
    // Network identity
    EntityId m_entityId{};
    
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
    
    // Hit reaction state
    bool m_isHit{false};
    float m_hitTimer{0.0f};
    float m_hitDuration{0.3f};  // Default hit animation duration (3 frames * 0.1s)
    
    // Health
    int m_health{100};
    int m_maxHealth{100};
    bool m_hasBeenDamaged{false};
    
    // State
    bool m_isAlive{true};
};

