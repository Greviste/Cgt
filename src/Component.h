#ifndef CGT_COMPONENT_H
#define CGT_COMPONENT_H

#include "WeakRef.hpp"
#include "Entity.h"
#include <tuple>

class Entity;
struct EntityKey;

class Component : public WeakReferencable
{
public:
    Component(const EntityKey& key);
    virtual ~Component() = default;

    Entity& owner() const;
private:
    Entity* _owner = nullptr;
};

template<typename T, typename... Args>
concept oneOf = (std::is_same_v<T, Args> || ...);

template<typename... Comps>
class DependentComponent : public Component
{
public:
    DependentComponent(const EntityKey& key)
        : Component(key), _dependencies{ &owner().getOrBuildComponent<Comps>()... }
    {
    }

    template<oneOf<Comps...> T>
    T& get() const
    {
        return *std::get<T*>(_dependencies);
    }

private:
    std::tuple<Comps*...> _dependencies;
};

#endif
