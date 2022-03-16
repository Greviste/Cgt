#ifndef CGT_ENTITY_H
#define CGT_ENTITY_H

#include "WeakRef.hpp"
#include "World.h"
#include <vector>
#include <memory>
#include <concepts>

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
public:
    Entity(const Entity&) = delete;
    Entity(Entity&&) = delete;
    ~Entity();

    template<std::derived_from<Component> T, typename... Args>
    T& buildComponent(Args&&... args)
    {
        auto uptr = std::make_unique<T>(EntityKey{ this }, std::forward<Args>(args)...);
        T& ref = *uptr;
        _components.push_back(std::move(uptr));
        _world->registerComponent(ref);
        return ref;
    }

    template<std::derived_from<Component> T>
    T* findComponent()
    {
        //For now, it'll be ugly
        for (auto& uptr : _components)
        {
            if (T* ptr = dynamic_cast<T*>(uptr.get()); ptr) return ptr;
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
    std::vector<std::unique_ptr<Component>> _components;
};

#endif
