#include "NetMessage.h"
#include <stdexcept>

namespace NetMessage {

// ============== PlayerInput ==============

Json::Value PlayerInput::ToJson() const {
    return Json::MakeObject()
        .Add("playerId", static_cast<int64_t>(playerId.value))
        .Add("moveX", static_cast<int>(moveX))
        .Add("moveY", static_cast<int>(moveY))
        .Add("targetTileX", targetTileX)
        .Add("targetTileY", targetTileY)
        .Add("attack", attack)
        .Add("facing", static_cast<int>(facing))
        .Add("clientTick", static_cast<int64_t>(clientTick))
        .Build();
}

PlayerInput PlayerInput::FromJson(const Json::Value& json) {
    PlayerInput input;
    input.playerId = EntityId(static_cast<uint32_t>(json["playerId"].AsInt()));
    input.moveX = static_cast<int8_t>(json["moveX"].AsInt());
    input.moveY = static_cast<int8_t>(json["moveY"].AsInt());
    input.targetTileX = static_cast<int>(json["targetTileX"].AsInt());
    input.targetTileY = static_cast<int>(json["targetTileY"].AsInt());
    input.attack = json["attack"].AsBool();
    input.facing = static_cast<Direction>(json["facing"].AsInt());
    input.clientTick = static_cast<uint32_t>(json["clientTick"].AsInt());
    return input;
}

// ============== ActionRequest ==============

Json::Value ActionRequest::ToJson() const {
    return Json::MakeObject()
        .Add("entityId", static_cast<int64_t>(entityId.value))
        .Add("action", static_cast<int>(action))
        .Add("targetId", static_cast<int64_t>(targetId.value))
        .Add("targetX", targetX)
        .Add("targetY", targetY)
        .Build();
}

ActionRequest ActionRequest::FromJson(const Json::Value& json) {
    ActionRequest req;
    req.entityId = EntityId(static_cast<uint32_t>(json["entityId"].AsInt()));
    req.action = static_cast<ActionRequest::ActionType>(json["action"].AsInt());
    req.targetId = EntityId(static_cast<uint32_t>(json["targetId"].AsInt()));
    req.targetX = static_cast<int>(json["targetX"].AsInt());
    req.targetY = static_cast<int>(json["targetY"].AsInt());
    return req;
}

// ============== EntitySpawn ==============

Json::Value EntitySpawn::ToJson() const {
    return Json::MakeObject()
        .Add("entityId", static_cast<int64_t>(entityId.value))
        .Add("entityType", static_cast<int>(entityType))
        .Add("tileX", tileX)
        .Add("tileY", tileY)
        .Add("health", health)
        .Add("maxHealth", maxHealth)
        .Add("facing", static_cast<int>(facing))
        .Add("typeId", typeId)
        .Build();
}

EntitySpawn EntitySpawn::FromJson(const Json::Value& json) {
    EntitySpawn spawn;
    spawn.entityId = EntityId(static_cast<uint32_t>(json["entityId"].AsInt()));
    spawn.entityType = static_cast<EntityId::Type>(json["entityType"].AsInt());
    spawn.tileX = static_cast<int>(json["tileX"].AsInt());
    spawn.tileY = static_cast<int>(json["tileY"].AsInt());
    spawn.health = static_cast<int>(json["health"].AsInt());
    spawn.maxHealth = static_cast<int>(json["maxHealth"].AsInt());
    spawn.facing = static_cast<Direction>(json["facing"].AsInt());
    spawn.typeId = json["typeId"].AsString();
    return spawn;
}

// ============== EntityDespawn ==============

Json::Value EntityDespawn::ToJson() const {
    return Json::MakeObject()
        .Add("entityId", static_cast<int64_t>(entityId.value))
        .Add("reason", static_cast<int>(reason))
        .Build();
}

EntityDespawn EntityDespawn::FromJson(const Json::Value& json) {
    EntityDespawn despawn;
    despawn.entityId = EntityId(static_cast<uint32_t>(json["entityId"].AsInt()));
    despawn.reason = static_cast<EntityDespawn::Reason>(json["reason"].AsInt());
    return despawn;
}

// ============== EntityUpdate ==============

Json::Value EntityUpdate::ToJson() const {
    return Json::MakeObject()
        .Add("entityId", static_cast<int64_t>(entityId.value))
        .Add("tileX", tileX)
        .Add("tileY", tileY)
        .Add("renderX", static_cast<double>(renderX))
        .Add("renderY", static_cast<double>(renderY))
        .Add("health", health)
        .Add("facing", static_cast<int>(facing))
        .Add("isMoving", isMoving)
        .Add("isPunching", isPunching)
        .Add("punchProgress", static_cast<double>(punchProgress))
        .Build();
}

EntityUpdate EntityUpdate::FromJson(const Json::Value& json) {
    EntityUpdate update;
    update.entityId = EntityId(static_cast<uint32_t>(json["entityId"].AsInt()));
    update.tileX = static_cast<int>(json["tileX"].AsInt());
    update.tileY = static_cast<int>(json["tileY"].AsInt());
    update.renderX = static_cast<float>(json["renderX"].AsFloat());
    update.renderY = static_cast<float>(json["renderY"].AsFloat());
    update.health = static_cast<int>(json["health"].AsInt());
    update.facing = static_cast<Direction>(json["facing"].AsInt());
    update.isMoving = json["isMoving"].AsBool();
    update.isPunching = json["isPunching"].AsBool();
    update.punchProgress = static_cast<float>(json["punchProgress"].AsFloat());
    return update;
}

// ============== EntityMove ==============

Json::Value EntityMove::ToJson() const {
    return Json::MakeObject()
        .Add("entityId", static_cast<int64_t>(entityId.value))
        .Add("fromX", fromX)
        .Add("fromY", fromY)
        .Add("toX", toX)
        .Add("toY", toY)
        .Add("isDiagonal", isDiagonal)
        .Build();
}

EntityMove EntityMove::FromJson(const Json::Value& json) {
    EntityMove move;
    move.entityId = EntityId(static_cast<uint32_t>(json["entityId"].AsInt()));
    move.fromX = static_cast<int>(json["fromX"].AsInt());
    move.fromY = static_cast<int>(json["fromY"].AsInt());
    move.toX = static_cast<int>(json["toX"].AsInt());
    move.toY = static_cast<int>(json["toY"].AsInt());
    move.isDiagonal = json["isDiagonal"].AsBool();
    return move;
}

// ============== EntityDamage ==============

Json::Value EntityDamage::ToJson() const {
    return Json::MakeObject()
        .Add("targetId", static_cast<int64_t>(targetId.value))
        .Add("attackerId", static_cast<int64_t>(attackerId.value))
        .Add("damage", damage)
        .Add("remainingHealth", remainingHealth)
        .Add("isCritical", isCritical)
        .Build();
}

EntityDamage EntityDamage::FromJson(const Json::Value& json) {
    EntityDamage dmg;
    dmg.targetId = EntityId(static_cast<uint32_t>(json["targetId"].AsInt()));
    dmg.attackerId = EntityId(static_cast<uint32_t>(json["attackerId"].AsInt()));
    dmg.damage = static_cast<int>(json["damage"].AsInt());
    dmg.remainingHealth = static_cast<int>(json["remainingHealth"].AsInt());
    dmg.isCritical = json["isCritical"].AsBool();
    return dmg;
}

// ============== EntityDeath ==============

Json::Value EntityDeath::ToJson() const {
    return Json::MakeObject()
        .Add("entityId", static_cast<int64_t>(entityId.value))
        .Add("killerId", static_cast<int64_t>(killerId.value))
        .Build();
}

EntityDeath EntityDeath::FromJson(const Json::Value& json) {
    EntityDeath death;
    death.entityId = EntityId(static_cast<uint32_t>(json["entityId"].AsInt()));
    death.killerId = EntityId(static_cast<uint32_t>(json["killerId"].AsInt()));
    return death;
}

// ============== WorldSnapshot ==============

Json::Value WorldSnapshot::ToJson() const {
    Json::ArrayBuilder entitiesArray;
    for (const auto& entity : entities) {
        entitiesArray.Add(entity.ToJson());
    }
    
    return Json::MakeObject()
        .Add("serverTick", static_cast<int64_t>(serverTick))
        .Add("entities", entitiesArray.Build())
        .Build();
}

WorldSnapshot WorldSnapshot::FromJson(const Json::Value& json) {
    WorldSnapshot snapshot;
    snapshot.serverTick = static_cast<uint32_t>(json["serverTick"].AsInt());
    
    const auto& entitiesJson = json["entities"].AsArray();
    snapshot.entities.reserve(entitiesJson.size());
    for (const auto& entityJson : entitiesJson) {
        snapshot.entities.push_back(EntitySpawn::FromJson(entityJson));
    }
    
    return snapshot;
}

// ============== Handshake ==============

Json::Value Handshake::ToJson() const {
    return Json::MakeObject()
        .Add("phase", static_cast<int>(phase))
        .Add("protocolVersion", static_cast<int>(protocolVersion))
        .Add("playerName", playerName)
        .Add("assignedPlayerId", static_cast<int64_t>(assignedPlayerId.value))
        .Build();
}

Handshake Handshake::FromJson(const Json::Value& json) {
    Handshake hs;
    hs.phase = static_cast<Handshake::Phase>(json["phase"].AsInt());
    hs.protocolVersion = static_cast<uint16_t>(json["protocolVersion"].AsInt());
    hs.playerName = json["playerName"].AsString();
    hs.assignedPlayerId = EntityId(static_cast<uint32_t>(json["assignedPlayerId"].AsInt()));
    return hs;
}

// ============== Message ==============

std::string Message::Serialize() const {
    Json::ObjectBuilder builder;
    
    // Add header
    builder.Add("type", static_cast<int>(header.type));
    builder.Add("seq", static_cast<int64_t>(header.sequenceNumber));
    builder.Add("ts", static_cast<int64_t>(header.timestamp));
    
    // Add data based on variant type
    std::visit([&builder](const auto& msg) {
        builder.Add("data", msg.ToJson());
    }, data);
    
    return builder.Build().Stringify();
}

std::optional<Message> Message::Deserialize(const std::string& jsonStr) {
    try {
        auto json = Json::Value::Parse(jsonStr);
        
        Message msg;
        msg.header.type = static_cast<Type>(json["type"].AsInt());
        msg.header.sequenceNumber = static_cast<uint32_t>(json["seq"].AsInt());
        msg.header.timestamp = static_cast<uint32_t>(json["ts"].AsInt());
        
        const auto& dataJson = json["data"];
        
        switch (msg.header.type) {
            case Type::PlayerInput:
                msg.data = PlayerInput::FromJson(dataJson);
                break;
            case Type::ActionRequest:
                msg.data = ActionRequest::FromJson(dataJson);
                break;
            case Type::EntitySpawn:
                msg.data = EntitySpawn::FromJson(dataJson);
                break;
            case Type::EntityDespawn:
                msg.data = EntityDespawn::FromJson(dataJson);
                break;
            case Type::EntityUpdate:
                msg.data = EntityUpdate::FromJson(dataJson);
                break;
            case Type::EntityMove:
                msg.data = EntityMove::FromJson(dataJson);
                break;
            case Type::EntityDamage:
                msg.data = EntityDamage::FromJson(dataJson);
                break;
            case Type::EntityDeath:
                msg.data = EntityDeath::FromJson(dataJson);
                break;
            case Type::WorldSnapshot:
                msg.data = WorldSnapshot::FromJson(dataJson);
                break;
            case Type::Handshake:
                msg.data = Handshake::FromJson(dataJson);
                break;
            default:
                return std::nullopt;
        }
        
        return msg;
    } catch (...) {
        return std::nullopt;
    }
}

} // namespace NetMessage
