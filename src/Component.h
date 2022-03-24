#ifndef CGT_COMPONENT_H
#define CGT_COMPONENT_H

#include "WeakRef.hpp"
#include "Entity.h"
#include <tuple>

class Entity;
struct EntityKey;
template<typename... Comps>
class DependentComponent;

class Component : public WeakReferencable
{
    template<typename... Comps>
    friend class DependentComponent;
    friend class Entity;
    friend class World;
public:
    Component(const EntityKey& key);
    virtual ~Component() = default;

    void destroy();
    Entity& owner() const;
    World& world() const;
private:
    void checkDestroy();
    bool shouldDestroy();

    Entity* _owner = nullptr;
    unsigned short _clients = 0;
    bool _marked_for_destroy = false;
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
        (++std::get<Comps*>(_dependencies)->_clients , ...);
    }

    ~DependentComponent()
    {
        (--std::get<Comps*>(_dependencies)->_clients , ...);
        (std::get<Comps*>(_dependencies)->checkDestroy() , ...);
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
