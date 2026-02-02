#pragma once

#include "EntityId.h"
#include "../Core/Direction.h"
#include "Json.h"
#include <cstdint>
#include <string>
#include <vector>
#include <variant>
#include <optional>

// Network message types for client-server communication
// 
// Architecture: Messages follow a command pattern where:
// - Clients send input/action messages to the server
// - Server validates and processes actions, then broadcasts state updates
// - State updates are authoritative and clients must apply them

namespace NetMessage {

// Message type identifiers for serialization
enum class Type : uint16_t {
    // Client -> Server: Input/Actions
    PlayerInput = 0x0100,          // Player movement/action input
    ActionRequest = 0x0101,        // Request to perform an action (attack, use item)
    
    // Server -> Client: State Updates
    EntitySpawn = 0x0200,          // New entity spawned
    EntityDespawn = 0x0201,        // Entity removed from world
    EntityUpdate = 0x0202,         // Entity state changed
    EntityMove = 0x0203,           // Entity position changed
    EntityDamage = 0x0204,         // Entity took damage
    EntityDeath = 0x0205,          // Entity died
    
    // Server -> Client: World State
    WorldSnapshot = 0x0300,        // Full world state (for initial sync)
    MapData = 0x0301,              // Map tile data
    
    // Bidirectional: Connection Management
    Handshake = 0x0400,            // Connection handshake
    Heartbeat = 0x0401,            // Keep-alive ping
    Disconnect = 0x0402,           // Graceful disconnect
    
    // Server -> Client: Game Events
    CombatEvent = 0x0500,          // Combat-related event (hit, miss, crit)
    GameEvent = 0x0501             // General game event
};

// Base structure for all network messages
struct Header {
    Type type;
    uint32_t sequenceNumber;   // For ordering and acknowledgment
    uint32_t timestamp;        // Server timestamp in milliseconds
    
    Header() : type(Type::Handshake), sequenceNumber(0), timestamp(0) {}
    Header(Type t, uint32_t seq, uint32_t ts) 
        : type(t), sequenceNumber(seq), timestamp(ts) {}
};

// ============== Client -> Server Messages ==============

// Player input state (sent every frame or on change)
struct PlayerInput {
    static constexpr Type kType = Type::PlayerInput;
    
    EntityId playerId;
    int8_t moveX;              // Movement direction X (-1, 0, 1)
    int8_t moveY;              // Movement direction Y (-1, 0, 1)
    int targetTileX;           // Click-to-move target (or -1 if not applicable)
    int targetTileY;
    bool attack;               // Attack button pressed
    Direction facing;          // Current facing direction
    uint32_t clientTick;       // Client's local tick for reconciliation
    
    PlayerInput() 
        : moveX(0), moveY(0)
        , targetTileX(-1), targetTileY(-1)
        , attack(false)
        , facing(Direction::South)
        , clientTick(0) {}
    
    [[nodiscard]] Json::Value ToJson() const;
    static PlayerInput FromJson(const Json::Value& json);
};

// Request to perform a specific action
struct ActionRequest {
    static constexpr Type kType = Type::ActionRequest;
    
    enum class ActionType : uint8_t {
        Attack,
        UseItem,
        Interact
    };
    
    EntityId entityId;
    ActionType action;
    EntityId targetId;         // Target entity (if applicable)
    int targetX, targetY;      // Target position (if applicable)
    
    [[nodiscard]] Json::Value ToJson() const;
    static ActionRequest FromJson(const Json::Value& json);
};

// ============== Server -> Client Messages ==============

// Entity spawned in the world
struct EntitySpawn {
    static constexpr Type kType = Type::EntitySpawn;
    
    EntityId entityId;
    EntityId::Type entityType;
    int tileX, tileY;
    int health, maxHealth;
    Direction facing;
    std::string typeId;        // For enemies: the enemy type ID
    
    [[nodiscard]] Json::Value ToJson() const;
    static EntitySpawn FromJson(const Json::Value& json);
};

// Entity removed from world
struct EntityDespawn {
    static constexpr Type kType = Type::EntityDespawn;
    
    EntityId entityId;
    enum class Reason : uint8_t {
        Death,
        Disconnect,
        OutOfRange
    } reason;
    
    [[nodiscard]] Json::Value ToJson() const;
    static EntityDespawn FromJson(const Json::Value& json);
};

// Entity state update (position, health, etc.)
struct EntityUpdate {
    static constexpr Type kType = Type::EntityUpdate;
    
    EntityId entityId;
    int tileX, tileY;
    float renderX, renderY;    // For interpolation
    int health;
    Direction facing;
    bool isMoving;
    bool isPunching;
    float punchProgress;
    
    [[nodiscard]] Json::Value ToJson() const;
    static EntityUpdate FromJson(const Json::Value& json);
};

// Entity moved to new position
struct EntityMove {
    static constexpr Type kType = Type::EntityMove;
    
    EntityId entityId;
    int fromX, fromY;
    int toX, toY;
    bool isDiagonal;
    
    [[nodiscard]] Json::Value ToJson() const;
    static EntityMove FromJson(const Json::Value& json);
};

// Entity took damage
struct EntityDamage {
    static constexpr Type kType = Type::EntityDamage;
    
    EntityId targetId;
    EntityId attackerId;
    int damage;
    int remainingHealth;
    bool isCritical;
    
    [[nodiscard]] Json::Value ToJson() const;
    static EntityDamage FromJson(const Json::Value& json);
};

// Entity died
struct EntityDeath {
    static constexpr Type kType = Type::EntityDeath;
    
    EntityId entityId;
    EntityId killerId;         // May be invalid if environmental death
    
    [[nodiscard]] Json::Value ToJson() const;
    static EntityDeath FromJson(const Json::Value& json);
};

// Full world snapshot (sent on join)
struct WorldSnapshot {
    static constexpr Type kType = Type::WorldSnapshot;
    
    uint32_t serverTick;
    std::vector<EntitySpawn> entities;
    
    [[nodiscard]] Json::Value ToJson() const;
    static WorldSnapshot FromJson(const Json::Value& json);
};

// Connection handshake
struct Handshake {
    static constexpr Type kType = Type::Handshake;
    
    enum class Phase : uint8_t {
        ClientHello,
        ServerChallenge,
        ClientResponse,
        ServerAccept
    };
    
    Phase phase;
    uint16_t protocolVersion;
    std::string playerName;
    EntityId assignedPlayerId; // Set by server in Accept phase
    
    [[nodiscard]] Json::Value ToJson() const;
    static Handshake FromJson(const Json::Value& json);
};

// ============== Generic Message Container ==============

// Variant type for all message types
using MessageData = std::variant<
    PlayerInput,
    ActionRequest,
    EntitySpawn,
    EntityDespawn,
    EntityUpdate,
    EntityMove,
    EntityDamage,
    EntityDeath,
    WorldSnapshot,
    Handshake
>;

// Complete message with header and data
struct Message {
    Header header;
    MessageData data;
    
    // Serialize to JSON string
    [[nodiscard]] std::string Serialize() const;
    
    // Deserialize from JSON string
    static std::optional<Message> Deserialize(const std::string& json);
};

} // namespace NetMessage
