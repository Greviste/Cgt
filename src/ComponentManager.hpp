#ifndef CGT_COMPONENTMANAGER_HPP
#define CGT_COMPONENTMANAGER_HPP

#include <vector>
#include <chrono>
#include <utility>
#include <glm/glm.hpp>
#include <forward_list>
#include "Utility.h"


class Entity;


using Seconds = std::chrono::duration<float>;

class ComponentManagerBase
{
public:
    virtual ~ComponentManagerBase() = default;
    virtual void update(Seconds delta) = 0;
    virtual void tick(Seconds delta) = 0;
    virtual void draw(const glm::mat4& v, const glm::mat4& p) const = 0;
    virtual void drawGeometry(const glm::mat4& v, const glm::mat4& p) const = 0;
    virtual void cleanup(Seconds delta) = 0;
protected:
};

template<typename C>
class ComponentManager : public ComponentManagerBase
{
public:
    void update(const Seconds delta) override
    {
        if constexpr (requires (C& c) { c.update(delta); })
        {
            for (auto& c : _components) if (!c.shouldDestroy()) c.update(delta);
        }
    }

    void tick(const Seconds delta) override
    {
        if constexpr (requires (C& c) { c.tick(delta); })
        {
            for (auto& c : _components) if (!c.shouldDestroy()) c.tick(delta);
        }
    }

    void draw(const glm::mat4& v, const glm::mat4& p) const override
    {
        if constexpr (requires (const C& c) { c.draw(v, p); })
        {
            for (auto& c : _components) c.draw(v, p);
        }
    }

    void drawGeometry(const glm::mat4& v, const glm::mat4& p) const override
    {
        if constexpr (requires (const C& c) { c.drawGeometry(v, p); })
        {
            for (auto& c : _components) c.drawGeometry(v, p);
        }
    }

    void cleanup(Seconds delta) override
    {
        using std::begin, std::end;
        _components.erase(Utility::forEachRemovable(_components, [](C& c) {
            if constexpr (requires (C& c) { c.stop(); }) if (c.shouldDestroy()) c.stop();
            return c.shouldDestroy();
        }), end(_components));

        auto list = std::move(_new_components);
        for (C& c : list)
        {
            if constexpr (requires (C& c) { c.start(); })
            {
                c.start();
            }
            if (c.shouldDestroy())
            {
                if constexpr (requires (C& c) { c.stop(); })
                {
                    c.stop();
                }
                continue;
            }
            auto it = _components.emplace(std::upper_bound(begin(_components), end(_components), &c.owner(), [](const Entity* l, const C& r) { return std::less<const Entity*>{}(l, &r.owner()); }), std::move(c));
            if constexpr (requires (C& c) { c.update(delta); })
            {
                it->update(delta);
            }
        }
    }

    template<typename... Args>
    C& build(Args&&... args)
    {
        _new_components.emplace_front(std::forward<Args>(args)...);
        return _new_components.front();
    }

    auto begin() { return _components.begin(); }
    auto begin() const { return _components.begin(); }
    auto cbegin() const { return _components.cbegin(); }
    auto end() { return _components.end(); }
    auto end() const { return _components.end(); }
    auto cend() const { return _components.cend(); }
private:
    std::vector<C> _components;
    std::forward_list<C> _new_components; //Needed to guarantee reference and pointer validity within a single update
};

#endif
