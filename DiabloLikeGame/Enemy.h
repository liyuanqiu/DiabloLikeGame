#pragma once

#include "Entity.h"

// Enemy entity
class Enemy : public Entity {
public:
    Enemy() = default;
    Enemy(int tileX, int tileY, int health);
    ~Enemy() override = default;
};
