#include "Entity.h"
#include "Component.h"

Entity::Entity(World& w)
    :_world(&w)
{
}
Entity::~Entity() = default;

void Entity::destroy()
{
    _marked_for_destroy = true;
    world()._should_cleanup = true;
}

World& Entity::world() const
{
    return *_world;
}
