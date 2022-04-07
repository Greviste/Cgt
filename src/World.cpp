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
    _tick_remainder += s;
    while (_tick_remainder >= _tick_period)
    {
        tickOnce();
        _tick_remainder -= _tick_period;
    }

    forEachManager([&s](ComponentManagerBase& m) { m.update(s); });

    while(_should_cleanup)
    {
        _should_cleanup = false;
        auto it = _entities.begin();
        erase_if(_entities, [](auto& ptr) { return ptr->_marked_for_destroy; });
        forEachManager([&s](ComponentManagerBase& m) { m.cleanup(s); });
    }
}

void World::draw(const glm::mat4& v, const glm::mat4& p) const
{
    for (auto& elem : _managers) elem.second->draw(v, p);
}

void World::tickOnce()
{
    forEachManager([=](ComponentManagerBase& m) { m.tick(_tick_period); });
}

template<typename F>
void World::forEachManager(F&& func)
{
    for (auto& elem : _managers) func(*elem.second);
    while (!_additional_managers.empty())
    {
        auto list = std::move(_additional_managers);
        for (auto& elem : list)
        {
            func(*elem.second);
            _managers.emplace(std::move(elem));
        }
    }
}
