#pragma once

#include "EntityId.h"
#include "NetMessage.h"
#include <memory>
#include <functional>
#include <optional>
#include <queue>
#include <vector>

// Forward declarations
class Entity;
class Player;
class Enemy;

// Interface for network state synchronization
// 
// Architecture: This abstraction allows the game to work in both single-player
// (local authority) and multiplayer (server authority) modes without changing
// game logic. The implementation decides whether to:
// - Execute actions immediately (single-player/server)
// - Send requests to server and wait for confirmation (client)
class INetworkAuthority {
public:
    virtual ~INetworkAuthority() = default;
    
    // Callback types
    using EntityCallback = std::function<void(EntityId)>;
    using DamageCallback = std::function<void(EntityId target, EntityId attacker, int damage, bool crit)>;
    using MoveCallback = std::function<void(EntityId entity, int toX, int toY)>;
    
    // ============== Authority Queries ==============
    
    // Returns true if this instance has authority over game state
    [[nodiscard]] virtual bool IsAuthority() const noexcept = 0;
    
    // Returns true if we should predict locally (client-side prediction)
    [[nodiscard]] virtual bool ShouldPredict() const noexcept = 0;
    
    // Get the local player's entity ID
    [[nodiscard]] virtual EntityId GetLocalPlayerId() const noexcept = 0;
    
    // Check if we have authority over a specific entity
    [[nodiscard]] virtual bool HasAuthorityOver(EntityId entityId) const noexcept = 0;
    
    // ============== Entity Lifecycle ==============
    
    // Request entity spawn (server decides actual ID)
    virtual EntityId RequestSpawn(EntityId::Type type, int tileX, int tileY) = 0;
    
    // Notify entity despawn
    virtual void NotifyDespawn(EntityId entityId, NetMessage::EntityDespawn::Reason reason) = 0;
    
    // ============== Movement ==============
    
    // Request entity movement (returns true if movement was initiated)
    virtual bool RequestMove(EntityId entityId, int toX, int toY) = 0;
    
    // Set path for click-to-move
    virtual void RequestPath(EntityId entityId, int destX, int destY) = 0;
    
    // ============== Combat ==============
    
    // Request an attack action
    virtual bool RequestAttack(EntityId attackerId, Direction facing) = 0;
    
    // Apply damage (server-authoritative)
    virtual void ApplyDamage(EntityId targetId, EntityId attackerId, int damage, bool isCrit) = 0;
    
    // ============== State Updates ==============
    
    // Send local player input to server
    virtual void SendPlayerInput(const NetMessage::PlayerInput& input) = 0;
    
    // Process incoming messages (call each frame)
    virtual void ProcessMessages() = 0;
    
    // Get pending state updates for entities
    [[nodiscard]] virtual std::optional<NetMessage::EntityUpdate> GetEntityUpdate(EntityId entityId) = 0;
    
    // ============== Event Callbacks ==============
    
    // Set callbacks for network events
    virtual void SetOnEntitySpawn(EntityCallback callback) = 0;
    virtual void SetOnEntityDespawn(EntityCallback callback) = 0;
    virtual void SetOnEntityMove(MoveCallback callback) = 0;
    virtual void SetOnEntityDamage(DamageCallback callback) = 0;
    
    // ============== ID Generation ==============
    
    // Generate a new unique entity ID (server only)
    [[nodiscard]] virtual EntityId GenerateEntityId(EntityId::Type type) = 0;
    
    // ============== Tick Management ==============
    
    // Get current server tick (for synchronization)
    [[nodiscard]] virtual uint32_t GetServerTick() const noexcept = 0;
    
    // Get local tick (for client-side prediction)
    [[nodiscard]] virtual uint32_t GetLocalTick() const noexcept = 0;
    
    // Advance local tick
    virtual void AdvanceTick() noexcept = 0;
};

// Local/Single-player implementation of INetworkAuthority
// All actions are executed immediately with local authority
class LocalAuthority : public INetworkAuthority {
public:
    LocalAuthority();
    ~LocalAuthority() override = default;
    
    // ============== Authority Queries ==============
    
    [[nodiscard]] bool IsAuthority() const noexcept override { return true; }
    [[nodiscard]] bool ShouldPredict() const noexcept override { return false; }
    [[nodiscard]] EntityId GetLocalPlayerId() const noexcept override { return m_localPlayerId; }
    [[nodiscard]] bool HasAuthorityOver(EntityId /*entityId*/) const noexcept override { return true; }
    
    // ============== Entity Lifecycle ==============
    
    EntityId RequestSpawn(EntityId::Type type, int tileX, int tileY) override;
    void NotifyDespawn(EntityId entityId, NetMessage::EntityDespawn::Reason reason) override;
    
    // ============== Movement ==============
    
    bool RequestMove(EntityId entityId, int toX, int toY) override;
    void RequestPath(EntityId entityId, int destX, int destY) override;
    
    // ============== Combat ==============
    
    bool RequestAttack(EntityId attackerId, Direction facing) override;
    void ApplyDamage(EntityId targetId, EntityId attackerId, int damage, bool isCrit) override;
    
    // ============== State Updates ==============
    
    void SendPlayerInput(const NetMessage::PlayerInput& input) override;
    void ProcessMessages() override;
    [[nodiscard]] std::optional<NetMessage::EntityUpdate> GetEntityUpdate(EntityId entityId) override;
    
    // ============== Event Callbacks ==============
    
    void SetOnEntitySpawn(EntityCallback callback) override { m_onSpawn = std::move(callback); }
    void SetOnEntityDespawn(EntityCallback callback) override { m_onDespawn = std::move(callback); }
    void SetOnEntityMove(MoveCallback callback) override { m_onMove = std::move(callback); }
    void SetOnEntityDamage(DamageCallback callback) override { m_onDamage = std::move(callback); }
    
    // ============== ID Generation ==============
    
    [[nodiscard]] EntityId GenerateEntityId(EntityId::Type type) override;
    
    // ============== Tick Management ==============
    
    [[nodiscard]] uint32_t GetServerTick() const noexcept override { return m_tick; }
    [[nodiscard]] uint32_t GetLocalTick() const noexcept override { return m_tick; }
    void AdvanceTick() noexcept override { ++m_tick; }
    
    // ============== Local Authority Specific ==============
    
    // Set the local player ID (called during initialization)
    void SetLocalPlayerId(EntityId id) noexcept { m_localPlayerId = id; }

private:
    EntityId m_localPlayerId;
    uint32_t m_tick{0};
    uint16_t m_nextEnemyInstance{1};
    uint16_t m_nextProjectileInstance{1};
    
    // Event callbacks
    EntityCallback m_onSpawn;
    EntityCallback m_onDespawn;
    MoveCallback m_onMove;
    DamageCallback m_onDamage;
    
    // Pending move requests (for path following)
    struct PendingPath {
        EntityId entityId;
        int destX, destY;
    };
    std::queue<PendingPath> m_pendingPaths;
};

// Factory for creating network authority instances
class NetworkAuthorityFactory {
public:
    enum class Mode {
        Local,          // Single-player, local authority
        DedicatedServer,// Server in client-server model
        Client          // Client in client-server model (not yet implemented)
    };
    
    // Create appropriate authority implementation
    [[nodiscard]] static std::unique_ptr<INetworkAuthority> Create(Mode mode);
    
    // Get singleton instance (lazy initialization with Local mode)
    [[nodiscard]] static INetworkAuthority& GetInstance();
    
    // Set mode before first GetInstance() call
    static void SetMode(Mode mode);

private:
    static std::unique_ptr<INetworkAuthority> s_instance;
    static Mode s_mode;
};
