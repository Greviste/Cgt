#include "World.h"

Entity& World::createEntity()
{
    _entities.emplace_back(new Entity{});
    return *_entities.back();
}
