#ifndef CGT_WORLD_H
#define CGT_WORLD_H

#include <vector>
#include <memory>
#include "Entity.h"

class World
{
public:
    Entity& createEntity();
private:
    std::vector<std::unique_ptr<Entity>> _entities;
};

#endif
