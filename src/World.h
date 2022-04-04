#ifndef CGT_WORLD_H
#define CGT_WORLD_H

#include <vector>
#include <unordered_map>
#include <memory>
#include <type_traits>
#include <typeinfo>
#include <typeindex>
#include <utility>
#include "ComponentManager.hpp"

class Entity;
class Component;

class World
{
    friend class Entity;
    friend class Component;
public:
    Entity& createEntity();
    void update(Seconds);
    void draw(const glm::mat4& v, const glm::mat4& p) const;
private:
    template<std::derived_from<Component> T, typename... Args>
    T& buildComponent(Args&&... args)
    {
        _should_cleanup = true;
        ComponentManagerBase* manager;
        if (auto it = _managers.find(typeid(T)); it != _managers.end())
        {
            manager = it->second.get();
        }
        else if(auto it = std::find_if(begin(_additional_managers), end(_additional_managers), [](auto& item) { return item.first == typeid(T); }); it != end(_additional_managers))
        {
            manager = it->second.get();
        }
        else
        {
            _additional_managers.emplace_back(typeid(T), std::make_unique<ComponentManager<T>>());
            manager = _additional_managers.back().second.get();
        }

        return static_cast<ComponentManager<T>*>(manager)->build(std::forward<Args>(args)...);
    }

    template<typename F>
    void forEachManager(F&& func); //Only used in the cpp, does not need to be defined in the header

    std::vector<std::unique_ptr<Entity>> _entities;
    std::unordered_map<std::type_index, std::unique_ptr<ComponentManagerBase>> _managers;
    std::vector<std::pair<const std::type_index, std::unique_ptr<ComponentManagerBase>>> _additional_managers; //Needed to prevent rehash in the middle of the update
    bool _should_cleanup = false;
};

#endif
