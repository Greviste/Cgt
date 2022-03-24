#include "Component.h"
#include "Entity.h"

Component::Component(const EntityKey& key)
    : _owner(key.ptr)
{
}

void Component::destroy()
{
    _marked_for_destroy = true;
    checkDestroy();
}

Entity& Component::owner() const
{
    return *_owner;
}

World& Component::world() const
{
    return _owner->world();
}

void Component::checkDestroy()
{
    if(shouldDestroy()) world()._should_cleanup = true;
}

bool Component::shouldDestroy()
{
    return _marked_for_destroy && _clients == 0;
}
