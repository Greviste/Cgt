#include "Entity.h"
#include "Component.h"

Entity::Entity(World& w)
    :_world(&w)
{
}
Entity::~Entity() = default;

World& Entity::world() const
{
    return *_world;
}
