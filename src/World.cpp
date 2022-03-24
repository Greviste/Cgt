#include "World.h"
#include "Entity.h"
#include "Component.h"

Entity& World::createEntity()
{
    _entities.emplace_back(new Entity(*this));
    return *_entities.back();
}

void World::update(Seconds s)
{
    for (auto u : _updateables) u->update(s);

    while(_should_cleanup)
    {
        _should_cleanup = false;
        auto it = _entities.begin();
        while(it != _entities.end())
        {
            if((*it)->_marked_for_destroy)
            {
                it = _entities.erase(it);
            }
            else
            {
                auto cit = (*it)->_components.begin();
                while(cit != (*it)->_components.end())
                {
                    if((*cit)->shouldDestroy()) cit = (*it)->_components.erase(cit);
                    else ++cit;
                }
                ++it;
            }
        }
    }
}

void World::draw(const glm::mat4& v, const glm::mat4& p) const
{
    for (auto d : _drawables) d->draw(v, p);
}
