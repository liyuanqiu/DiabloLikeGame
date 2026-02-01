#include "Enemy.h"

Enemy::Enemy(int tileX, int tileY, int health)
    : Entity(tileX, tileY)
{
    SetHealth(health, health);
}
