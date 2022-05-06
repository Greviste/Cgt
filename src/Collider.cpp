#include "Collider.h"
#include "Physics.h"
#include <glm/gtx/norm.hpp>

void CollisionVolume::start()
{
    Physics::instance().add(*this);
}

void CollisionVolume::stop()
{
    Physics::instance().remove(*this);
}

namespace
{
    Sphere build(const CollisionSphere& s, const Transformation& t)
    {
        return { t.worldPosition(), s.radius };
    }
    Obb build(const CollisionBox& s, const Transformation& t)
    {
        return { t.worldPosition(), t.worldRotation(), s.extents };
    }
}

AnyCol CollisionVolume::buildCollisions() const
{
    return std::visit<AnyCol>([&](auto& x) { return build(x, get<Transformation>()); }, volume);
}


void PhysicsMovement::start()
{
    Physics::instance().add(*this);
}

void PhysicsMovement::stop()
{
    Physics::instance().remove(*this);
}

void PhysicsMovement::tick(Seconds delta)
{
    using std::cos, std::sin;
    Transformation& t = get<Transformation>();
    CollisionVolume& cv = get<CollisionVolume>();

    glm::vec3 forces = gravity * mass - drag * _velocity;
    _velocity += forces / mass * delta.count();
    _angular_velocity *= 1 - angular_drag;

    if (length2(_angular_velocity) > 0)
    {
        glm::vec3 rotation_vector = _angular_velocity * delta.count();
        float rotation_length = length(rotation_vector);
        glm::quat rotation{ cos(rotation_length / 2), rotation_vector / rotation_length * sin(rotation_length / 2) };
        glm::quat old_rot = t.rotation();
        t.rotation(rotation * old_rot);
        if (Physics::instance().overlap(cv.buildCollisions()).size() > 1) //1 for self overlap
        {
            t.rotation(old_rot);
        }
    }


    glm::vec3 movement = _velocity * delta.count();
    while (auto result = Physics::instance().sweep(cv, movement))
    {
        t.translation(t.translation() + movement * result->t + result->normal * TinyLength);
        handleBounce(*result, result->other_physics);
        if (result->other_physics) result->other_physics->handleBounce(*result, this);
        delta *= 1 - result->t;
        movement = _velocity * delta.count();
    }
    t.translation(t.translation() + movement);
}

glm::vec3 PhysicsMovement::velocity() const
{
    return _velocity;
}

void PhysicsMovement::velocity(glm::vec3 v)
{
    _velocity = v;
}

void PhysicsMovement::handleBounce(const Intersection& intersection, PhysicsMovement* other)
{
    float denom = 1 / mass;
    if (other) denom += 1 / other->mass;
    glm::vec3 v = _velocity;
    if (other) v -= other->_velocity;

    denom *= mass;
    glm::vec3 diff = (1 + cr) * dot(v, intersection.normal) / denom * intersection.normal;
    _angular_velocity -= cross(intersection.contact - get<Transformation>().translation(), diff);
    _velocity -= diff;
}
