#include "Component.h"
#include "Entity.h"

Component::Component(const EntityKey& key)
    : _owner(key.ptr)
{
}

Entity& Component::owner() const
{
    return *_owner;
}
