#ifndef CGT_GEOMETRY_H
#define CGT_GEOMETRY_H
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <optional>

struct Segment
{
    glm::vec3 a, b;
};

struct Sphere
{
    glm::vec3 center;
    float radius;
};

struct Capsule
{
    Segment segment;
    float radius;
};

struct Aabb
{
    glm::vec3 center;
    glm::vec3 extents;
};

struct Obb
{
    glm::vec3 center;
    glm::quat rotation;
    glm::vec3 extents;
};

struct Intersection
{
    glm::vec3 contact;
    glm::vec3 normal;
    float t;
};

float projectionCoord(const glm::vec3& x, const glm::vec3& on);
float projectionCoord(const glm::vec3& p, const Segment& s);
glm::vec3 lerp(const glm::vec3& a, const glm::vec3& b, float t);
float sqr(float x);
float sign(float x);

bool intersect(const Sphere& l, const Sphere& r);
bool intersect(const Capsule& l, const Sphere& r);
bool intersect(const Sphere& l, const Capsule& r);
bool intersect(const Capsule& l, const Capsule& r);
bool intersect(const Aabb& l, const Sphere& r);
bool intersect(const Sphere& l, const Aabb& r);
bool intersect(const Aabb& l, const Capsule& r);
bool intersect(const Capsule& l, const Aabb& r);
bool intersect(const Aabb& l, const Aabb& r);
bool intersect(const Obb& l, const Sphere& r);
bool intersect(const Sphere& l, const Obb& r);
bool intersect(const Obb& l, const Capsule& r);
bool intersect(const Capsule& l, const Obb& r);
bool intersect(const Obb& l, const Aabb& r);
bool intersect(const Aabb& l, const Obb& r);
bool intersect(const Obb& l, const Obb& r);

std::optional<Intersection> intersectMoving(const Sphere& l, const Sphere& r, glm::vec3 movement);
std::optional<Intersection> intersectMoving(const Aabb& l, const Aabb& r, glm::vec3 movement);
std::optional<Intersection> intersectMoving(const Obb& l, const Obb& r, glm::vec3 movement);
std::optional<Intersection> intersectMoving(const Aabb& l, Sphere r, glm::vec3 movement);
std::optional<Intersection> intersectMoving(const Sphere& l, const Aabb& r, const glm::vec3& movement);
std::optional<Intersection> intersectMoving(const Obb& l, Sphere r, const glm::vec3& movement);
std::optional<Intersection> intersectMoving(const Sphere& l, const Obb& r, const glm::vec3& movement);

#endif
