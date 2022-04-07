#ifndef CGT_PHYSICS_H
#define CGT_PHYSICS_H

#include <vector>
#include <utility>
#include <variant>
#include <unordered_map>
#include "WeakRef.hpp"
#include "Geometry.h"

class Entity;
class CollisionVolume;
class PhysicsMovement;

struct SweepResult : public Intersection
{
    CollisionVolume* other;
    PhysicsMovement* other_physics;
};

class Physics
{
public:
    static Physics& instance();
    std::optional<SweepResult> sweep(const AnyCol& col, const glm::vec3& movement) const;

    void add(CollisionVolume&);
    void remove(CollisionVolume&);
    void add(PhysicsMovement&);
    void remove(PhysicsMovement&);
private:
    enum struct Status
    {
        None, Static, Dynamic
    };

    using DynamicInfo = std::pair<WeakRef<CollisionVolume>, WeakRef<PhysicsMovement>>;
    std::vector<WeakRef<CollisionVolume>> _statics;
    std::vector<DynamicInfo> _dynamics;
    std::unordered_map<Entity*, Status> _status;
};

#endif
