#ifndef CGT_COLLIDER_H
#define CGT_COLLIDER_H

#include "Component.h"
#include "Transformation.h"
#include "Event.hpp"
#include "Geometry.h"
#include <variant>
#include <vector>

struct CollisionSphere
{
    float radius;
};

struct CollisionBox
{
    glm::vec3 extents;
};

class CollisionVolume : public DependentComponent<Transformation>
{
public:
    using DependentComponent::DependentComponent;

    void start();
    void stop();

    AnyCol buildCollisions() const;

    std::variant<CollisionSphere, CollisionBox> volume;
private:
};

class PhysicsMovement : public DependentComponent<Transformation,CollisionVolume>
{
public:
    using DependentComponent::DependentComponent;

    void start();
    void stop();
    void tick(Seconds delta);

    glm::vec3 velocity() const;
    void velocity(glm::vec3 v);

    glm::vec3 gravity{ 0, -9.81f, 0 };
    float cr = 0.8f;
    float drag = 0.01f;
    float angular_drag = 0.01f;
    float mass = 10.f;
private:
    void handleBounce(const Intersection& intersection, PhysicsMovement* other);
    glm::vec3 _velocity{};
    glm::vec3 _angular_velocity{};
};

#endif
