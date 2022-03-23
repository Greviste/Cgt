#include "World.h"
#include "Entity.h"

Entity& World::createEntity()
{
    _entities.emplace_back(new Entity(*this));
    return *_entities.back();
}

void World::update(Seconds s)
{
    for (auto u : _updateables) u->update(s);
}

void World::draw(const glm::mat4& v, const glm::mat4& p)
{
    for (auto d : _drawables) d->draw(v, p);
}
