#ifndef CGT_ENTITY_H
#define CGT_ENTITY_H

#include "WeakRef.hpp"
#include "World.h"
#include <unordered_map>
#include <concepts>
#include <typeinfo>
#include <typeindex>

class Component;
struct EntityKey
{
    friend class Entity;
    friend class Component;
private:
    EntityKey(Entity* p) : ptr(p) {}
    ~EntityKey() = default;
    Entity* ptr;
};

class Entity : public WeakReferencable
{
    friend class World;
    friend class Component;
public:
    Entity(const Entity&) = delete;
    Entity(Entity&&) = delete;
    ~Entity();

    void destroy();

    template<std::derived_from<Component> T, typename... Args>
    T& buildComponent(Args&&... args)
    {
        T& ref = _world->buildComponent<T>(EntityKey{ this }, std::forward<Args>(args)...);
        _components.emplace(typeid(T), ref);
        return ref;
    }

    template<std::derived_from<Component> T>
    T* findComponent() const
    {
        auto [it, sentinel] = _components.equal_range(typeid(T));
        while (it != sentinel)
        {
            if (it->second) return static_cast<T*>(it->second.ptr());
            it = _components.erase(it);
        }
        return nullptr;
    }

    template<std::derived_from<Component> T>
    T& getOrBuildComponent()
    {
        if (T* ptr = findComponent<T>(); ptr) return *ptr;
        return buildComponent<T>();
    }

    World& world() const;
private:
    Entity(World&);

    World* _world;
    mutable std::unordered_multimap<std::type_index, WeakRef<Component>> _components;
    bool _marked_for_destroy = false;
};

#endif
