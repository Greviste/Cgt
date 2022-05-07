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
    using std::cos, std::sin, std::abs;
    Transformation& t = get<Transformation>();
    CollisionVolume& cv = get<CollisionVolume>();

    glm::vec3 forces = gravity * mass - drag * _velocity;

    auto col = cv.buildCollisions();
    for (auto* other_cv : Physics::instance().overlap(growBy(col, TinyLength * 5)))
    {
        if (other_cv == &cv) continue;
        if (auto result = intersectMoving(other_cv->buildCollisions(), col, other_cv->get<Transformation>().translation() - t.translation()))
        {
            glm::vec3 normal_force = -dot(forces, result->normal) * result->normal;
            forces += normal_force;
            glm::vec3 bitangent = cross(normal_force, _velocity);
            glm::vec3 tangent_force{};
            if (length2(bitangent) > 0)
            {
                tangent_force = -0.1f * cross(normalize(bitangent), normal_force);
                forces += tangent_force;
            }
            glm::vec3 r = result->contact - t.translation();
            _angular_velocity += cross(r, normal_force + tangent_force) * delta.count();
        }
    }

    glm::vec3 origin = t.translation();
    _velocity += forces / mass * delta.count();
    glm::vec3 movement = _velocity * delta.count();
    while (auto result = Physics::instance().sweep(cv, movement))
    {
        t.translation(t.translation() + movement * result->t);
        glm::vec3 other_pos = result->other->get<Transformation>().translation();
        auto other_col = result->other->buildCollisions();
        do
        {
            auto bf = t.translation();
            t.translation(t.translation() + result->normal * sign(dot(t.translation() - other_pos, result->normal)) * TinyLength);
        } while (intersect(cv.buildCollisions(), other_col));

        handleBounce(*result, result->other_physics);
        delta *= 1 - result->t;
        movement = _velocity * delta.count();
    }
    t.translation(t.translation() + movement);
    glm::vec3 total_movement = t.translation() - origin;

    if (length2(_angular_velocity) > 0)
    {
        _angular_velocity *= 1 - angular_drag;
        glm::vec3 rotation_vector = _angular_velocity * delta.count();
        float rotation_length = length(rotation_vector);
        glm::quat old_rot = t.rotation();
        glm::quat rotation{ cos(rotation_length / 2), rotation_vector / rotation_length * sin(rotation_length / 2) };
        rotation *= old_rot;
        float f = 1;
        float best_f = 0;
        int step = 0;
        CollisionVolume* last_touched = nullptr;
        CollisionVolume* bounce_target = nullptr;
        do
        {
            last_touched = nullptr;
            t.translation(origin + total_movement * f);
            t.rotation(glm::slerp(old_rot, rotation, f));
            for (auto* other_cv : Physics::instance().overlap(cv.buildCollisions()))
            {
                if (other_cv == &cv) continue;
                last_touched = other_cv;
                break;
            }
            float f_change = 1.f / (1 << ++step);
            if (last_touched)
            {
                bounce_target = last_touched;
                f -= f_change;
            }
            else
            {
                best_f = f;
                f += f_change;
            }
        } while (step < 4 && f < 1);
        t.translation(origin + total_movement * best_f);
        t.rotation(glm::slerp(old_rot, rotation, best_f));
        if (bounce_target && length2(total_movement) > 0)
        {
            glm::vec3 dir = normalize(total_movement);
            Intersection inter{ getFarthestPoint(cv.buildCollisions(), dir), dir, best_f };
            handleBounce(inter, nullptr);
        }
    }
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
    auto [diff, angular_diff] = calcBounceDiff(intersection, other);
    if (other)
    {
        auto [odiff, oangular_diff] = other->calcBounceDiff(intersection, this);
        other->_velocity += odiff;
        other->_angular_velocity += _angular_velocity;
    }
    _velocity += diff;
    _angular_velocity += angular_diff;
}

std::pair<glm::vec3, glm::vec3> PhysicsMovement::calcBounceDiff(const Intersection& intersection, PhysicsMovement* other) const
{
    glm::vec3 r = intersection.contact - get<Transformation>().translation();
    float denom = (1 + dot(intersection.normal, cross(cross(r, intersection.normal), r))) / mass;
    glm::vec3 v = _velocity;
    if (other)
    {
        glm::vec3 rb = intersection.contact - other->get<Transformation>().translation();
        denom += (1 + dot(intersection.normal, cross(cross(rb, intersection.normal), rb))) / other->mass;
        v -= other->_velocity;
    }

    denom *= mass;
    glm::vec3 diff = -(1 + cr) * dot(v, intersection.normal) / denom * intersection.normal;

    return { diff, cross(r, diff) };
}
