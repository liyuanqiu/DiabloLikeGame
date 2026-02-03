#pragma once

#include <cstdint>
#include <functional>

// Unique identifier for entities in the game world
// Used for network synchronization and entity lookup
// 
// Architecture note: For multiplayer, each entity needs a unique ID that can be
// serialized and transmitted over the network. The ID consists of:
// - Type bits (2): Entity type for quick filtering
// - Owner bits (14): Player/client ID who owns/controls this entity (0 = server)
// - Instance bits (16): Unique instance number within the owner's scope
struct EntityId {
    // Entity types for quick filtering
    enum class Type : uint8_t {
        Invalid = 0,
        Player = 1,
        Enemy = 2,
        Projectile = 3
    };
    
    // The raw 32-bit ID value
    uint32_t value = 0;
    
    // Default constructor creates invalid ID
    EntityId() = default;
    
    // Construct from raw value
    explicit EntityId(uint32_t raw) noexcept : value(raw) {}
    
    // Construct from components
    // Note: owner is limited to 14 bits (max value 16383)
    EntityId(Type type, uint16_t owner, uint16_t instance) noexcept {
        // Mask owner to 14 bits (max 16383)
        value = (static_cast<uint32_t>(type) << 30) |
                (static_cast<uint32_t>(owner & 0x3FFF) << 16) |
                static_cast<uint32_t>(instance);
    }
    
    // Component accessors
    [[nodiscard]] Type GetType() const noexcept {
        return static_cast<Type>((value >> 30) & 0x3);
    }
    
    [[nodiscard]] uint16_t GetOwner() const noexcept {
        return static_cast<uint16_t>((value >> 16) & 0x3FFF);
    }
    
    [[nodiscard]] uint16_t GetInstance() const noexcept {
        return static_cast<uint16_t>(value & 0xFFFF);
    }
    
    // Validity check
    [[nodiscard]] bool IsValid() const noexcept {
        return value != 0 && GetType() != Type::Invalid;
    }
    
    // Comparison operators
    [[nodiscard]] bool operator==(const EntityId& other) const noexcept {
        return value == other.value;
    }
    
    [[nodiscard]] bool operator!=(const EntityId& other) const noexcept {
        return value != other.value;
    }
    
    [[nodiscard]] bool operator<(const EntityId& other) const noexcept {
        return value < other.value;
    }
    
    // Create an invalid ID
    [[nodiscard]] static EntityId Invalid() noexcept {
        return EntityId{};
    }
    
    // Create a player entity ID
    [[nodiscard]] static EntityId CreatePlayer(uint16_t playerId) noexcept {
        return EntityId(Type::Player, playerId, playerId);
    }
    
    // Create an enemy entity ID (server-owned)
    [[nodiscard]] static EntityId CreateEnemy(uint16_t instanceId) noexcept {
        return EntityId(Type::Enemy, 0, instanceId);
    }
    
    // Create a projectile entity ID
    [[nodiscard]] static EntityId CreateProjectile(uint16_t owner, uint16_t instanceId) noexcept {
        return EntityId(Type::Projectile, owner, instanceId);
    }
};

// Hash function for use in std::unordered_map/set
namespace std {
    template<>
    struct hash<EntityId> {
        size_t operator()(const EntityId& id) const noexcept {
            return std::hash<uint32_t>{}(id.value);
        }
    };
}
