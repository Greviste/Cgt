#ifndef CGT_WORLD_H
#define CGT_WORLD_H

#include <vector>
#include <memory>
#include <type_traits>
#include "BaseInterfaces.h"

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
    template<typename T>
    void registerComponent(T& component)
    {
        if constexpr (std::is_convertible_v<T*, Updateable*>)
        {
            _updateables.push_back(&component);
        }
        if constexpr (std::is_convertible_v<T*, Drawable*>)
        {
            _drawables.push_back(&component);
        }
    }

    std::vector<std::unique_ptr<Entity>> _entities;
    std::vector<Updateable*> _updateables;
    std::vector<Drawable*> _drawables;
    bool _should_cleanup = false;
};

#endif
