#include "NetworkAuthority.h"
#include <stdexcept>

// ============== LocalAuthority Implementation ==============

LocalAuthority::LocalAuthority()
    : m_localPlayerId(EntityId::CreatePlayer(1))  // Default player ID
{
}

EntityId LocalAuthority::RequestSpawn(EntityId::Type type, int /*tileX*/, int /*tileY*/) {
    // Generate ID and notify
    EntityId id = GenerateEntityId(type);
    if (m_onSpawn) {
        m_onSpawn(id);
    }
    return id;
}

void LocalAuthority::NotifyDespawn(EntityId entityId, NetMessage::EntityDespawn::Reason /*reason*/) {
    if (m_onDespawn) {
        m_onDespawn(entityId);
    }
}

bool LocalAuthority::RequestMove(EntityId entityId, int toX, int toY) {
    // Local authority: movement is always approved
    if (m_onMove) {
        m_onMove(entityId, toX, toY);
    }
    return true;
}

void LocalAuthority::RequestPath(EntityId entityId, int destX, int destY) {
    // Queue path request for processing
    m_pendingPaths.push({entityId, destX, destY});
}

bool LocalAuthority::RequestAttack(EntityId /*attackerId*/, Direction /*facing*/) {
    // Local authority: attack is always approved
    return true;
}

void LocalAuthority::ApplyDamage(EntityId targetId, EntityId attackerId, int damage, bool isCrit) {
    if (m_onDamage) {
        m_onDamage(targetId, attackerId, damage, isCrit);
    }
}

void LocalAuthority::SendPlayerInput(const NetMessage::PlayerInput& /*input*/) {
    // Local authority: input is processed immediately, no need to send
}

void LocalAuthority::ProcessMessages() {
    // Local authority: no network messages to process
    // Process any pending paths
    while (!m_pendingPaths.empty()) {
        // Path requests are handled by the game logic directly
        m_pendingPaths.pop();
    }
}

std::optional<NetMessage::EntityUpdate> LocalAuthority::GetEntityUpdate(EntityId /*entityId*/) {
    // Local authority: no server updates to apply
    return std::nullopt;
}

EntityId LocalAuthority::GenerateEntityId(EntityId::Type type) {
    switch (type) {
        case EntityId::Type::Player:
            // Players get their ID from connection management
            return m_localPlayerId;
            
        case EntityId::Type::Enemy:
            return EntityId::CreateEnemy(m_nextEnemyInstance++);
            
        case EntityId::Type::Projectile:
            return EntityId::CreateProjectile(
                m_localPlayerId.GetOwner(), 
                m_nextProjectileInstance++
            );
            
        default:
            return EntityId::Invalid();
    }
}

// ============== NetworkAuthorityFactory Implementation ==============

std::unique_ptr<INetworkAuthority> NetworkAuthorityFactory::s_instance = nullptr;
NetworkAuthorityFactory::Mode NetworkAuthorityFactory::s_mode = NetworkAuthorityFactory::Mode::Local;

std::unique_ptr<INetworkAuthority> NetworkAuthorityFactory::Create(Mode mode) {
    switch (mode) {
        case Mode::Local:
        case Mode::DedicatedServer:
            // Both local and server use LocalAuthority (server has full authority)
            return std::make_unique<LocalAuthority>();
            
        case Mode::Client:
            // TODO: Implement ClientAuthority
            throw std::runtime_error("Client network authority not yet implemented");
            
        default:
            throw std::runtime_error("Unknown network authority mode");
    }
}

void NetworkAuthorityFactory::SetMode(Mode mode) {
    if (s_instance) {
        throw std::runtime_error("Cannot change mode after GetInstance() has been called");
    }
    s_mode = mode;
}

INetworkAuthority& NetworkAuthorityFactory::GetInstance() {
    if (!s_instance) {
        s_instance = Create(s_mode);
    }
    return *s_instance;
}
