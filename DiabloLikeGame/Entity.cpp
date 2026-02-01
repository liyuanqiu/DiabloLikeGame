#include "Entity.h"
#include <algorithm>

Entity::Entity(int tileX, int tileY)
    : m_tileX(tileX)
    , m_tileY(tileY)
    , m_renderX(static_cast<float>(tileX))
    , m_renderY(static_cast<float>(tileY))
    , m_health(100)
    , m_maxHealth(100)
    , m_hasBeenDamaged(false)
    , m_isAlive(true)
{
}

void Entity::SetTilePosition(int tileX, int tileY) noexcept
{
    m_tileX = tileX;
    m_tileY = tileY;
}

void Entity::SetRenderPosition(float renderX, float renderY) noexcept
{
    m_renderX = renderX;
    m_renderY = renderY;
}

void Entity::SetHealth(int health, int maxHealth) noexcept
{
    m_maxHealth = std::max(1, maxHealth);
    m_health = std::clamp(health, 0, m_maxHealth);
}

void Entity::TakeDamage(int amount) noexcept
{
    if (amount <= 0 || !m_isAlive) return;
    
    m_hasBeenDamaged = true;
    m_health = std::max(0, m_health - amount);
    
    if (m_health <= 0) {
        m_isAlive = false;
    }
}

void Entity::Heal(int amount) noexcept
{
    if (amount <= 0 || !m_isAlive) return;
    
    m_health = std::min(m_maxHealth, m_health + amount);
}

void Entity::FaceToward(int targetX, int targetY) noexcept
{
    const int dx = targetX - m_tileX;
    const int dy = targetY - m_tileY;
    
    // Clamp to -1, 0, 1 for direction
    const int ndx = (dx > 0) ? 1 : (dx < 0) ? -1 : 0;
    const int ndy = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;
    
    // Only update if there's a direction
    if (ndx != 0 || ndy != 0) {
        m_facing = DirectionUtil::FromDelta(ndx, ndy);
    }
}

void Entity::StartPunch() noexcept
{
    if (!m_isPunching && m_isAlive) {
        m_isPunching = true;
        m_punchProgress = 0.0f;
        m_punchHitProcessed = false;
    }
}

void Entity::UpdatePunch(float deltaTime) noexcept
{
    if (!m_isPunching) return;
    
    // Progress the punch animation
    m_punchProgress += deltaTime / m_punchDuration;
    
    if (m_punchProgress >= 1.0f) {
        // Punch complete
        m_isPunching = false;
        m_punchProgress = 0.0f;
    }
}
