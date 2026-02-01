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
