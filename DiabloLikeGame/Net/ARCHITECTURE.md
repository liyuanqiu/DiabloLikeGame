# Multiplayer Architecture Design

This document describes the architecture for supporting multiplayer online gameplay in DiabloLikeGame.

## Overview

The game uses a **server-authoritative** architecture where:
- The server has final authority over all game state
- Clients send input to the server
- Server validates and processes actions, then broadcasts updates
- Clients apply server updates and may use prediction for responsiveness

## Key Components

### 1. Entity Identification (`Net/EntityId.h`)

Every entity in the game has a unique 32-bit identifier that can be serialized for network transmission.

```cpp
// EntityId structure:
// [2 bits: Type][14 bits: Owner][16 bits: Instance]

EntityId id = EntityId::CreatePlayer(playerId);    // Player entities
EntityId id = EntityId::CreateEnemy(instanceNum);  // Server-owned enemies
EntityId id = EntityId::CreateProjectile(owner, instance);  // Projectiles
```

**Benefits:**
- Fast lookup and comparison (single 32-bit value)
- Type information for quick filtering
- Owner information for authority checks
- Unique across network

### 2. Network Messages (`Net/NetMessage.h`)

All game state changes are represented as messages that can be serialized to JSON for network transmission.

**Client → Server Messages:**
- `PlayerInput`: Movement direction, attack state, facing
- `ActionRequest`: Specific actions (attack, use item, interact)

**Server → Client Messages:**
- `EntitySpawn`: New entity created
- `EntityDespawn`: Entity removed (death, disconnect)
- `EntityUpdate`: Full state update for an entity
- `EntityMove`: Position change
- `EntityDamage`: Damage applied
- `EntityDeath`: Entity died
- `WorldSnapshot`: Full world state (for initial sync)

**Bidirectional:**
- `Handshake`: Connection establishment
- `Heartbeat`: Keep-alive

### 3. Network Authority (`Net/NetworkAuthority.h`)

The `INetworkAuthority` interface abstracts whether actions are executed locally or sent to a server.

```cpp
// Single-player uses LocalAuthority (immediate execution)
// Multiplayer client uses ClientAuthority (send to server)
// Server uses ServerAuthority (process and broadcast)

INetworkAuthority& auth = NetworkAuthorityFactory::GetInstance();

// Request movement (same API regardless of mode)
if (auth.RequestMove(entityId, newX, newY)) {
    // Movement approved (or prediction initiated)
}
```

**Authority Modes:**
- `Local`: Single-player, all actions execute immediately
- `DedicatedServer`: Server instance, processes client input
- `Client`: Multiplayer client, sends input to server

### 4. Game Data Client (`Net/IGameDataClient.h`)

Abstracts configuration data retrieval:
- `GameDataLocalClient`: Reads from local INI files
- Future `GameDataNetworkClient`: Fetches from server

## Data Flow

### Single-Player Mode

```
[Input] → [Game Logic] → [Entity State] → [Renderer]
```

All processing happens locally with `LocalAuthority`.

### Multiplayer Mode (Client)

```
[Input] → [Client Authority] → [Network] → [Server]
                                              ↓
[Renderer] ← [Entity State] ← [Server Updates]
```

1. Client captures input and sends `PlayerInput` message
2. Server validates and processes the input
3. Server broadcasts `EntityUpdate` messages
4. Client applies server state to local entities

### Multiplayer Mode (Server)

```
[Client Input] → [Validation] → [Game Logic] → [Broadcast Updates]
                                     ↓
                              [World State]
```

1. Receive client input messages
2. Validate actions (anti-cheat, rate limiting)
3. Process game logic (same as single-player)
4. Generate and broadcast state update messages

## Entity State Synchronization

### Tick-Based Updates

The game runs on a fixed tick rate (e.g., 20 ticks/second for server updates):

```cpp
// Each message includes server tick for ordering
uint32_t serverTick = auth.GetServerTick();
```

### Interpolation

Clients interpolate between received states for smooth rendering:

```cpp
// Entity has both logical and render positions
int tileX, tileY;        // Server-authoritative position
float renderX, renderY;  // Interpolated for smooth animation
```

### Client-Side Prediction (Future)

For responsive controls, clients can predict local player actions:

1. Execute action locally immediately
2. Send action to server
3. When server response arrives:
   - If matches: Continue normally
   - If differs: Reconcile to server state

## State Serialization

All state uses JSON for serialization:

```cpp
// Serialize entity spawn
NetMessage::EntitySpawn spawn;
spawn.entityId = id;
spawn.tileX = x;
spawn.tileY = y;
std::string json = spawn.ToJson().Stringify();

// Deserialize
auto parsed = NetMessage::EntitySpawn::FromJson(Json::Value::Parse(json));
```

## Combat in Multiplayer

Combat uses server-authoritative hit detection:

1. Client sends attack input with facing direction
2. Server determines if attack hits (position, timing)
3. Server calculates damage (with randomization)
4. Server broadcasts `EntityDamage` and `EntityDeath` messages
5. Clients update local state from server messages

## Connection Flow

1. **Handshake**: Client connects and identifies itself
2. **World Sync**: Server sends `WorldSnapshot` with all entities
3. **Gameplay**: Client sends input, server broadcasts updates
4. **Disconnect**: Graceful or timeout-based cleanup

## Future Improvements

### Phase 1 (Current)
- [x] Entity ID system
- [x] Network message types
- [x] Authority abstraction
- [x] JSON serialization

### Phase 2
- [ ] Actual network transport (WebSocket/TCP)
- [ ] Client-side prediction
- [ ] Lag compensation
- [ ] Server instance management

### Phase 3
- [ ] Multiple game rooms
- [ ] Matchmaking
- [ ] Persistent player data
- [ ] Anti-cheat measures

## Code Organization

```
DiabloLikeGame/
├── Net/
│   ├── EntityId.h          # Unique entity identification
│   ├── NetMessage.h/.cpp   # Network message types
│   ├── NetworkAuthority.h/.cpp  # Authority abstraction
│   ├── IGameDataClient.h   # Data client interface
│   ├── GameDataLocalClient.h/.cpp  # Local implementation
│   ├── GameDataClientFactory.h/.cpp  # Factory
│   └── Json.h/.cpp         # JSON serialization
├── Entity.h/.cpp           # Base entity with EntityId
├── Player.h/.cpp           # Player entity
└── Enemy.h/.cpp            # Enemy entity
```

## Testing Strategy

### Unit Tests
- EntityId generation and comparison
- Message serialization/deserialization
- Authority mode switching

### Integration Tests
- Full game loop with LocalAuthority
- State synchronization accuracy

### Network Tests (Future)
- Message ordering
- Packet loss handling
- Reconnection
