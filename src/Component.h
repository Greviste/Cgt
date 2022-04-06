#ifndef CGT_COMPONENT_H
#define CGT_COMPONENT_H

#include "WeakRef.hpp"
#include "Entity.h"
#include <tuple>

class Entity;
struct EntityKey;
template<typename... Comps>
class DependentComponent;
template<typename C>
class ComponentManager;

class Component : public WeakReferencable
{
    template<typename... Comps>
    friend class DependentComponent;
    template<typename C>
    friend class ComponentManager;
    friend class Entity;
    friend class World;
public:
    Component(const EntityKey& key);
    void destroy();
    Entity& owner() const;
    World& world() const;

protected:
    Component(Component&&) = default;
    Component& operator=(Component&&) = default;
    ~Component() = default;

private:
    void checkDestroy();
    bool shouldDestroy();

    WeakRef<Entity> _owner;
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
        : Component(key), _dependencies{ owner().getOrBuildComponent<Comps>()... }
    {
    }
protected:
    DependentComponent(DependentComponent&&) = default;
    DependentComponent& operator=(DependentComponent&&) = default;

    template<oneOf<Comps...> T>
    T& get() const
    {
        return *std::get<DependentRef<T>>(_dependencies)._ref;
    }
    ~DependentComponent() = default;

private:
    template<typename T>
    struct DependentRef
    {
        DependentRef(T& ref) :_ref(ref) { ++ref._clients; }
        ~DependentRef()
        {
            if (!_ref) return;
            --_ref->_clients;
            _ref->checkDestroy();
        }

        DependentRef(DependentRef&& other)
            :_ref(std::move(other._ref))
        {
        }
        DependentRef& operator=(DependentRef&& other)
        {
            using std::swap;
            DependentRef copy = std::move(other);
            swap(_ref, copy._ref);
            return *this;
        }

        WeakRef<T> _ref;
    };
    std::tuple<DependentRef<Comps>...> _dependencies;
};

#endif
