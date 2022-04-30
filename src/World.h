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

struct LightData
{
    glm::vec3 pos; //+0
    glm::vec3 color; //+1
    float intensity; //+2
    glm::mat4 vp; //+3
    unsigned shadowMap; //+4
};

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
    void drawGeometry(const glm::mat4& v, const glm::mat4& p) const;
    template<typename T>
    auto getAll()
    {
        return doGetAll<T>();
    }

    template<typename T>
    auto getAll() const
    {
        return doGetAll<const T>();
    }

    const std::vector<LightData>& lightData() const;

private:
    template<typename T>
    std::vector<T*> doGetAll() const
    {
        using RawT = std::remove_cv_t<T>;
        std::vector<T*> result;
        if (ComponentManager<RawT>* manager = findManager<RawT>())
        {
            auto b = manager->begin();
            auto e = manager->end();
            result.resize(e - b);
            std::transform(b, e, result.begin(), [](auto& c) {return &c; });
        }
        return result;
    }

    template<typename T>
    ComponentManager<T>* findManager() const
    {
        if (auto it = _managers.find(typeid(T)); it != _managers.end())
        {
            return static_cast<ComponentManager<T>*>(it->second.get());
        }
        else if (auto it = std::find_if(begin(_additional_managers), end(_additional_managers), [](auto& item) { return item.first == typeid(T); }); it != end(_additional_managers))
        {
            return static_cast<ComponentManager<T>*>(it->second.get());
        }
        return nullptr;
    }

    template<std::derived_from<Component> T, typename... Args>
    T& buildComponent(Args&&... args)
    {
        _should_cleanup = true;
        ComponentManagerBase* manager = findManager<T>();
        if(!manager)
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

    mutable std::vector<LightData> _lights;
};

#endif
