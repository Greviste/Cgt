#include "World.h"
#include "Entity.h"
#include "Component.h"
#include <algorithm>

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
                    if ((*cit)->shouldDestroy())
                    {
                        Component* ptr = cit->get();
                        if (Drawable* drawable = dynamic_cast<Drawable*>(ptr))
                        {
                            _drawables.erase(std::remove(_drawables.begin(), _drawables.end(), drawable), _drawables.end());
                        }
                        if (Updateable* updateable = dynamic_cast<Updateable*>(ptr))
                        {
                            _updateables.erase(std::remove(_updateables.begin(), _updateables.end(), updateable), _updateables.end());
                        }
                        cit = (*it)->_components.erase(cit);
                    }
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
