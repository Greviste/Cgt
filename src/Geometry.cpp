#include "Geometry.h"
#include <glm/gtx/norm.hpp>
#include <cmath>
#include <algorithm>

float projectionCoord(const glm::vec3& x, const glm::vec3& on)
{
    return dot(x, on) / length2(on);
}

float projectionCoord(const glm::vec3& p, const Segment& s)
{
    return projectionCoord(p - s.a, s.b - s.a);
}

glm::vec3 lerp(const glm::vec3& a, const glm::vec3& b, float t)
{
    return a + (b - a) * t;
}

float sqr(float x)
{
    return x * x;
}

float sign(float x)
{
    return x == 0 ? 0 : std::copysign(1.f, x);
}

//Sphere
bool intersect(const Sphere& l, const Sphere& r)
{
    float radsum = l.radius + r.radius;
    float sqrdist = distance2(l.center, r.center);
    if (sqrdist > radsum * radsum)
    {
        return false;
    }

    return true;
}

//Capsule
bool intersect(const Capsule& l, const Sphere& r)
{
    Sphere closest{ lerp(l.segment.a, l.segment.b, std::clamp(projectionCoord(r.center, l.segment), 0.f, 1.f)), l.radius };
    return intersect(closest, r);
}

bool intersect(const Sphere& l, const Capsule& r)
{
    return intersect(r, l);
}

bool intersect(const Capsule& l, const Capsule& r)
{
    glm::vec3 l_to_r = r.segment.a - l.segment.a;
    glm::vec3 l_segment_vec = l.segment.b - l.segment.a;
    glm::vec3 r_segment_vec = r.segment.b - r.segment.a;
    glm::vec3 orthogonal_vec = cross(l_segment_vec, r_segment_vec);

    if (length2(orthogonal_vec) == 0)
    {
        //float cla = projectionCoord(l.segment.a - l.segment.a, l_segment_vec);
        const float cla = 0;
        //float clb = projectionCoord(l_segment_vec, l_segment_vec);
        const float clb = 1;
        float cra = projectionCoord(r.segment.a - l.segment.a, l_segment_vec);
        float crb = projectionCoord(r.segment.b - l.segment.a, l_segment_vec);
        if (cra < 0 || crb < 0)
        {
            Sphere l_sphere{ l.segment.a, l.radius };
            return intersect(l_sphere, r);
        }
        if (cra > 1 || crb > 1)
        {
            Sphere l_sphere{ l.segment.b, l.radius };
            return intersect(l_sphere, r);
        }
        Sphere r_sphere{ r.segment.a, r.radius };
        return intersect(l, r_sphere);
    }

    glm::vec3 rejection = l_to_r - projectionCoord(l_to_r, l_segment_vec) * l_segment_vec - projectionCoord(l_to_r, orthogonal_vec) * orthogonal_vec;
    float closest_r_coord = -1 / projectionCoord(r_segment_vec, rejection);
    Sphere r_sphere{ r.segment.a + std::clamp(closest_r_coord, 0.f, 1.f) * r_segment_vec, r.radius };
    return intersect(l, r_sphere);
}

//Aabb
bool intersect(const Aabb& l, const Sphere& r)
{
    float dmin = 0;
    glm::vec3 mins = l.center - l.extents;
    glm::vec3 maxs = l.center + l.extents;
    for (int i = 0; i < 3; ++i)
    {
        if (float d = r.center[i] - mins[i]; d < 0)
        {
            dmin += d * d;
        }
        else if (float d = r.center[i] - maxs[i]; d > 0)
        {
            dmin += d * d;
        }
    }
    if (dmin > r.radius * r.radius) return false;
    return true;
}

bool intersect(const Sphere& l, const Aabb& r)
{
    return intersect(r, l);
}

bool intersect(const Aabb& l, const Capsule& r)
{
    Sphere closest{ lerp(r.segment.a, r.segment.b, std::clamp(projectionCoord(l.center, r.segment), 0.f, 1.f)), r.radius };
    return intersect(l, closest);
}

bool intersect(const Capsule& l, const Aabb& r)
{
    return intersect(r, l);
}

bool intersect(const Aabb& l, const Aabb& r)
{
    glm::vec3 l_mins = l.center - l.extents;
    glm::vec3 l_maxs = l.center + l.extents;
    glm::vec3 r_mins = r.center - r.extents;
    glm::vec3 r_maxs = r.center + r.extents;
    for (int i = 0; i < 3; ++i)
    {
        if (r_maxs[i] < l_mins[i] || r_mins[i] > l_maxs[i]) return false;
    }

    return true;
}

//Obb
bool intersect(const Obb& l, const Sphere& r)
{
    auto rot = conjugate(l.rotation);
    Sphere local_r{ rot * r.center, r.radius };
    return intersect(Aabb{rot * l.center, l.extents}, local_r);
}

bool intersect(const Sphere& l, const Obb& r)
{
    return intersect(r, l);
}

bool intersect(const Obb& l, const Capsule& r)
{
    auto rot = conjugate(l.rotation);
    Capsule local_r{ {rot * r.segment.a, rot * r.segment.b}, r.radius };
    return intersect(Aabb{ rot * l.center, l.extents }, local_r);
}

bool intersect(const Capsule& l, const Obb& r)
{
    return intersect(r, l);
}

bool intersect(const Obb& l, const Obb& r);

bool intersect(const Obb& l, const Aabb& r)
{
    return intersect(l, Obb{ r.center, glm::quat{1,0,0,0}, r.extents });
}

bool intersect(const Aabb& l, const Obb& r)
{
    return intersect(Obb{ l.center, glm::quat{1,0,0,0}, l.extents }, r);
}

bool intersect(const Obb& l, const Obb& r)
{
    const Obb* boxes[2] = { &l, &r };

    glm::vec3 axis[2][3];
    glm::vec3 normals[15];
    for (int b = 0; b < 2; ++b)
    {
        glm::quat rot = boxes[b]->rotation;
        for (int a = 0; a < 3; ++a)
        {
            glm::vec3 e{};
            e[a] = 1;
            normals[b * 3 + a] = axis[b][a] = rot * e;
        }
    }
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            normals[6 + i * 3 + j] = cross(axis[0][i], axis[1][j]);
        }
    }

    glm::vec3 vertices[2][8];
    for (int b = 0; b < 2; ++b)
    {
        for (int v = 0; v < 8; ++v)
        {
            vertices[b][v] = boxes[b]->center;
            for (int a = 0; a < 3; ++a)
            {
                vertices[b][v] += axis[b][a] * boxes[b]->extents[a] * (v& (1 << a) ? 1.f : -1.f);
            }
        }
    }

    for (auto& n : normals)
    {
        if (length2(n) == 0) continue;
        float mins[2] = {
            std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::infinity()
        };
        float maxs[2] = {
            -std::numeric_limits<float>::infinity(),
            -std::numeric_limits<float>::infinity()
        };
        for (int b = 0; b < 2; ++b)
        {
            for (auto& v : vertices[b])
            {
                float d = dot(n, v);
                mins[b] = std::min(mins[b], d);
                maxs[b] = std::max(maxs[b], d);
            }
        }
        if (mins[0] > maxs[1] || mins[1] > maxs[0]) return false;
    }

    return true;
}

//For movement intersections, l is always assumed to be static

std::optional<Intersection> intersectMoving(const Sphere& l, const Sphere& r, glm::vec3 movement)
{
    float radius = l.radius + r.radius;
    float d = dot(r.center - l.center, movement);
    float v2 = length2(movement);
    float delta = 4 * (sqr(d) - v2 * (distance2(l.center, r.center) - sqr(radius)));
    float t = (-d - 2 * std::sqrt(delta)) / (2 * v2);
    if (t < 0)
    {
        t = (-d + 2 * std::sqrt(delta)) / (2 * v2);
        if (t < 0) return std::nullopt;
    }
    if (t > 1) return std::nullopt;
    glm::vec3 r_pos = r.center + t * movement;
    glm::vec3 l_to_r = normalize(r_pos - l.center);
    return Intersection{ l.center + l.radius * l_to_r, l_to_r, t };
}

std::optional<Intersection> intersectMoving(const Aabb& l, const Aabb& r, glm::vec3 movement)
{
    using std::min, std::max, std::max_element, std::minmax;

    glm::vec3 u0;
    glm::vec3 u1;
    glm::vec3 l_mins = l.center - l.extents;
    glm::vec3 l_maxs = l.center + l.extents;
    glm::vec3 r_mins = r.center - r.extents;
    glm::vec3 r_maxs = r.center + r.extents;
    for (int a = 0; a < 3; ++a)
    {
        float v = movement[a];
        std::tie(u0[a], u1[a]) = minmax((l_maxs[a] - r_mins[a]) / v, (l_mins[a] - r_maxs[a]) / v);
    }
    int axis = max_element(&u0[0], &u0[0] + 3) - &u0[0];
    float t_min = u0[axis];
    float t_max = min({ u1[0], u1[1], u1[2] });
    if (t_min > 1 || t_max < 0 || t_min > t_max) return std::nullopt;
    float t = max(t_min, 0.f);
    glm::vec3 normal{};
    normal[axis] = l.center[axis] < r.center[axis] ? 1 : -1;
    glm::vec3 contact;
    for (int a = 0; a < 3; ++a)
    {
        if (a == axis)
        {
            contact[a] = l.center[a] + l.extents[a] * normal[a];
        }
        else
        {
            contact[a] = (max(l_mins[a], r_mins[a]) + min(l_maxs[a], r_maxs[a])) / 2;
        }
    }

    return Intersection{ contact, normal, t };
}

std::optional<Intersection> intersectMoving(const Obb& l, const Obb& r, glm::vec3 movement)
{
    using std::min, std::max, std::minmax;

    const Obb* boxes[2] = { &l, &r };

    glm::vec3 axis[2][3];
    glm::vec3 normals[15];
    glm::vec3 additional_normals[6];
    for (int b = 0; b < 2; ++b)
    {
        glm::quat rot = boxes[b]->rotation;
        for (int a = 0; a < 3; ++a)
        {
            int index = b * 3 + a;
            glm::vec3 e{};
            e[a] = 1;
            normals[index] = axis[b][a] = rot * e;
            additional_normals[index] = cross(movement, normals[index]);
        }
    }
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            normals[6 + i * 3 + j] = cross(axis[0][i], axis[1][j]);
        }
    }

    glm::vec3 vertices[2][8];
    for (int b = 0; b < 2; ++b)
    {
        for (int v = 0; v < 8; ++v)
        {
            vertices[b][v] = boxes[b]->center;
            for (int a = 0; a < 3; ++a)
            {
                vertices[b][v] += axis[b][a] * boxes[b]->extents[a] * (v & (1 << a) ? 1.f : -1.f);
            }
        }
    }

    float t_min = 0, t_max = 1;
    int latest_axis = 0;
    float direction = 1;
    for (int n = 0; n < 15; ++n)
    {
        if (length2(normals[n]) == 0) continue;
        float mins[2] = {
            std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::infinity()
        };
        float maxs[2] = {
            -std::numeric_limits<float>::infinity(),
            -std::numeric_limits<float>::infinity()
        };
        for (int b = 0; b < 2; ++b)
        {
            for (auto& v : vertices[b])
            {
                float d = dot(normals[n], v);
                mins[b] = min(mins[b], d);
                maxs[b] = max(maxs[b], d);
            }
        }
        float v = dot(normals[n], movement);
        if (v == 0)
        {
            if (maxs[0] < mins[1] || mins[0] > maxs[1]) return std::nullopt;
            continue;
        }

        auto [mi, ma] = minmax((maxs[0] - mins[1]) / v, (mins[0] - maxs[1]) / v);
        if (mi > t_min)
        {
            t_min = mi;
            latest_axis = n;
            direction = v < 0 ? 1 : -1;
        }
        t_max = min(t_max, ma);

        if (t_min > 1 || t_max < 0 || t_min > t_max) return std::nullopt;
    }
    for (int n = 0; n < 6; ++n)
    {
        if (length2(additional_normals[n]) == 0) continue;
        float mins[2] = {
            std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::infinity()
        };
        float maxs[2] = {
            -std::numeric_limits<float>::infinity(),
            -std::numeric_limits<float>::infinity()
        };
        for (int b = 0; b < 2; ++b)
        {
            for (auto& v : vertices[b])
            {
                float d = dot(additional_normals[n], v);
                mins[b] = min(mins[b], d);
                maxs[b] = max(maxs[b], d);
            }
        }
        if (maxs[0] < mins[1] || mins[0] > maxs[1]) return std::nullopt;
    }

    glm::vec3 r_center = r.center + movement * t_min;
    glm::vec3 offset = r_center - l.center;
    glm::vec3 collision_axis = normals[latest_axis];
    glm::vec3 normal = collision_axis * direction;
    float sigma = dot(offset, collision_axis) > 0 ? 1 : -1;
    glm::mat3 c = toMat3(conjugate(l.rotation) * r.rotation);
    glm::vec3 contact_coords;
    int contact_box;

    switch (latest_axis)
    {
    case 0:
    case 1:
    case 2:
        contact_box = 1;
        for (int j = 0; j < 3; ++j) contact_coords[j] = -sigma * sign(c[latest_axis][j]) * r.extents[j];
        break;
    case 3:
    case 4:
    case 5:
        contact_box = 0;
        for (int i = 0; i < 3; ++i) contact_coords[i] = sigma * sign(c[i][latest_axis-3]) * l.extents[i];
        break;
    default:
        contact_box = 0;
        for (int k = 0; k < 3; ++k)
        {
            int i = ((latest_axis - 6) / 3 + k + 1) % 3;
            int i1 = (i + 1) % 3;
            int i2 = (i + 2) % 3;
            int j = (latest_axis - 6) % 3;
            int j1 = (j + 1) % 3;
            int j2 = (j + 2) % 3;
            if (i == (latest_axis - 6) / 3)
            {
                float yj1 = -sigma * sign(c[i][j2]) * r.extents[j1];
                float yj2 = sigma * sign(c[i][j1]) * r.extents[j2];
                contact_coords[i] = 1 / (1 - sqr(c[i][j])) * (dot(axis[0][i], offset) + c[i][j] * (-dot(axis[1][j], offset) + c[i1][j] * contact_coords[i1] + c[i2][j] * contact_coords[i2]) + c[i][j1] * yj1 + c[i][j2] * yj2);
            }
            else
            {
                float s = k == 0 ? -1 : 1;
                contact_coords[i] = s * sign(c[((latest_axis - 6) / 3 + 2 - k) % 3][j]) * l.extents[i];
            }
        }
    }

    return Intersection{(contact_box == 0 ? l.center : r_center) + boxes[contact_box]->rotation * contact_coords, normal, t_min};
}

namespace
{
    Intersection faceOverlap(int axis, const glm::vec3& extents, const glm::vec3& center)
    {
        glm::vec3 n{};
        n[axis] = 1;
        return Intersection{ n * extents + (glm::vec3{1,1,1} - n) * center, n, 0 };
    }

    Intersection edgeOverlap(int axis, const glm::vec3& extents, const glm::vec3& center)
    {
        glm::vec3 n{};
        n[axis] = 1;
        glm::vec3 n_inv = glm::vec3{ 1,1,1 } - n;
        return Intersection{ n * center + n_inv * extents, normalize(n_inv), 0};
    }

    std::optional<Intersection> checkRoundedVertex(const glm::vec3& vertex, float radius, const glm::vec3& delta, const glm::vec3& movement)
    {
        float a1 = dot(delta, movement);
        if (a1 < 0)
        {
            float a0 = length2(delta) - sqr(radius);
            float a2 = length2(movement);
            float adiscr = sqr(a1) - a2 * a0;
            if (adiscr >= 0)
            {
                float t = -(a1 + std::sqrt(adiscr)) / a2;
                return Intersection{ vertex, -normalize(delta + t * movement), t };
            }
        }
        return std::nullopt;
    }

    std::optional<Intersection> checkRoundedEdge(int axis, const glm::vec3& extents, float radius, const glm::vec3& delta, const glm::vec3& movement)
    {
        glm::vec3 axisv{};
        axisv[axis] = 1;
        glm::vec3 axisv_inv = glm::vec3{ 1,1,1 } - axisv;
        float a = length2(movement * axisv_inv);
        float b = 2 * dot(delta * axisv_inv, movement * axisv_inv);
        float c = length2(delta * axisv_inv);
        float d = sqr(b) - 4 * a * c;
        if (d < 0) return std::nullopt;
        float t = (-b - sqrt(d)) / (2 * a);
        if (t < 0 || t > 1) return std::nullopt;
        glm::vec3 pos = extents + delta + movement * t;
        if (pos[axis] > extents[axis])
        {
            return checkRoundedVertex(extents, radius, delta, movement);
        }
        if (pos[axis] < -extents[axis])
        {
            return checkRoundedVertex(extents * axisv_inv - extents * axisv, radius, delta + 2.f * axisv * extents, movement);
        }
        glm::vec3 contact = extents * axisv_inv + pos * axisv;
        return Intersection{ contact, normalize(pos - contact), t };
    }

    std::optional<Intersection> checkRoundedFace(int axis, const glm::vec3& extents, float radius, const glm::vec3& delta, const glm::vec3& movement)
    {
        using std::abs;

        float t = -delta[axis] / movement[axis];
        if (t < 0 || t > 1) return std::nullopt;
        glm::vec3 contact = extents + delta + movement * t;
        for (int i = 0; i < 2; ++i)
        {
            int a = (axis + i + 1) % 3;
            int other_a = (axis + 2 - i) % 3;
            if (abs(contact[a]) > extents[a] + radius) return std::nullopt;
            if (abs(contact[a]) > extents[a])
            {
                if (contact[a] > 0) return checkRoundedEdge(other_a, extents, radius, delta, movement);
                glm::vec3 av{}; av[a] = 1;
                return checkRoundedEdge(other_a, extents - 2.f * av * extents, radius, delta + 2.f * av * extents, movement);
            }
        }
        glm::vec3 axisv{};
        axisv[axis] = 1;
        glm::vec3 axisv_inv = glm::vec3{ 1,1,1 } - axisv;
        return Intersection{ contact * axisv_inv + extents * axisv, axisv, t };
    }

    std::optional<Intersection> vertexSeparated(const glm::vec3& vertex, float radius, const glm::vec3& delta, const glm::vec3& movement)
    {
        if (movement[0] < 0 || movement[1] < 0 || movement[2] < 0) return checkRoundedVertex(vertex, radius, delta, movement);
        return std::nullopt;
    }

    std::optional<Intersection> edgeSeparated(int axis, const glm::vec3& extents, float radius, const glm::vec3& delta, const glm::vec3& movement)
    {
        if (movement[(axis + 1) % 3] < 0 || movement[(axis + 2) % 3] < 0) return checkRoundedEdge(axis, extents, radius, delta, movement);
        return std::nullopt;
    }

    std::optional<Intersection> vertexUnbounded(const glm::vec3& extents, float radius, const glm::vec3& delta, const glm::vec3& movement)
    {
        if (movement[0] >= 0 || movement[1] >= 0 || movement[2] >= 0) return std::nullopt;
        if (auto inter = checkRoundedFace(0, extents, radius, delta, movement)) return inter;
        if (auto inter = checkRoundedFace(1, extents, radius, delta, movement)) return inter;
        return checkRoundedFace(2, extents, radius, delta, movement);
    }

    std::optional<Intersection> edgeUnbounded(int axis, const glm::vec3& extents, float radius, const glm::vec3& delta, const glm::vec3& movement)
    {
        if (movement[(axis + 1) % 3] >= 0 || movement[(axis + 2) % 3] >= 0) return std::nullopt;
        if (auto inter = checkRoundedFace((axis + 1) % 3, extents, radius, delta, movement)) return inter;
        return checkRoundedFace((axis + 2) % 3, extents, radius, delta, movement);
    }

    std::optional<Intersection> faceUnbounded(int axis, const glm::vec3& extents, float radius, const glm::vec3& delta, const glm::vec3& movement)
    {
        if (movement[axis] >= 0) return std::nullopt;
        return checkRoundedFace(axis, extents, radius, delta, movement);
    }

    std::optional<Intersection> intersectMovingCanonic(const glm::vec3& extents, const glm::vec3& c, float r, const glm::vec3& movement)
    {
        glm::vec3 delta = c - extents;
        if (delta[0] <= 0 && delta[1] <= 0 && delta[2] <= 0) return Intersection{ c, normalize(c), 0 };
        if (delta[0] > 0 && delta[0] <= r && delta[1] <= 0 && delta[2] <= 0) return faceOverlap(0, extents, c);
        if (delta[0] <= 0 && delta[1] > 0 && delta[1] <= r && delta[2] <= 0) return faceOverlap(1, extents, c);
        if (delta[0] > 0 && delta[0] <= r && delta[1] > 0 && delta[1] <= r && delta[2] <= 0 && sqr(delta[0]) + sqr(delta[1]) <= sqr(r)) return edgeOverlap(2, extents, c);
        if (delta[0] > 0 && delta[0] <= r && delta[1] > 0 && delta[1] <= r && delta[2] <= 0 && sqr(delta[0]) + sqr(delta[1]) > sqr(r)) return edgeSeparated(2, extents, r, delta, movement);
        if (delta[0] <= 0 && delta[1] <= 0 && delta[2] > 0 && delta[2] <= r) return faceOverlap(2, extents, c);
        if (delta[0] > 0 && delta[0] <= r && delta[1] <= 0 && delta[2] > 0 && delta[2] <= r && sqr(delta[0]) + sqr(delta[2]) <= sqr(r)) return edgeOverlap(1, extents, c);
        if (delta[0] > 0 && delta[0] <= r && delta[1] <= 0 && delta[2] > 0 && delta[2] <= r && sqr(delta[0]) + sqr(delta[2]) > sqr(r)) return edgeSeparated(1, extents, r, delta, movement);
        if (delta[0] <= 0 && delta[1] > 0 && delta[1] <= r && delta[2] > 0 && delta[2] <= r && sqr(delta[1]) + sqr(delta[2]) <= sqr(r)) return edgeOverlap(0, extents, c);
        if (delta[0] <= 0 && delta[1] > 0 && delta[1] <= r && delta[2] > 0 && delta[2] <= r && sqr(delta[1]) + sqr(delta[2]) > sqr(r)) return edgeSeparated(0, extents, r, delta, movement);
        if (delta[0] > 0 && delta[0] <= r && delta[1] > 0 && delta[1] <= r && delta[2] > 0 && delta[2] <= r && sqr(delta[0]) + sqr(delta[1]) + sqr(delta[2]) <= sqr(r)) return Intersection{extents, normalize(delta), 0};
        if (delta[0] > 0 && delta[0] <= r && delta[1] > 0 && delta[1] <= r && delta[2] > 0 && delta[2] <= r && sqr(delta[0]) + sqr(delta[1]) + sqr(delta[2]) > sqr(r)) return vertexSeparated(extents, r, delta, movement);
        if (delta[0] > r && delta[1] <= r && delta[2] <= r) return faceUnbounded(0, extents, r, delta, movement);
        if (delta[0] <= r && delta[1] > r && delta[2] <= r) return faceUnbounded(1, extents, r, delta, movement);
        if (delta[0] > r && delta[1] > r && delta[2] <= r) return edgeUnbounded(2, extents, r, delta, movement);
        if (delta[0] <= r && delta[1] <= r && delta[2] > r) return faceUnbounded(2, extents, r, delta, movement);
        if (delta[0] > r && delta[1] <= r && delta[2] > r) return edgeUnbounded(1, extents, r, delta, movement);
        if (delta[0] <= r && delta[1] > r && delta[2] > r) return edgeUnbounded(0, extents, r, delta, movement);
        //if (delta[0] > r && delta[1] > r && delta[2] > r)
        return vertexUnbounded(extents, r, delta, movement);
    }

    template<typename T, typename U>
    std::optional<Intersection> swapAndIntersect(const T& l, const U& r, const glm::vec3& movement)
    {
        std::optional<Intersection> result = intersectMoving(r, l, -movement);
        if (result)
        {
            result->normal *= -1;
        }
        return result;
    }
}

std::optional<Intersection> intersectMoving(const Aabb& l, Sphere r, glm::vec3 movement)
{
    r.center -= l.center;
    glm::vec3 inversions;
    for (int a = 0; a < 3; ++a) inversions[a] = r.center[a] < 0 ? -1 : 1;
    r.center *= inversions;
    movement *= inversions;
    auto result = intersectMovingCanonic(l.extents, r.center, r.radius, movement);
    if (result)
    {
        result->contact = result->contact * inversions + l.center;
        result->normal *= inversions;
    }
    return result;
}

std::optional<Intersection> intersectMoving(const Sphere& l, const Aabb& r, const glm::vec3& movement)
{
    return swapAndIntersect(l, r, movement);
}

std::optional<Intersection> intersectMoving(const Obb& l, Sphere r, const glm::vec3& movement)
{
    auto inv_rot = conjugate(l.rotation);
    r.center = inv_rot * r.center;
    auto result = intersectMoving(Aabb{ inv_rot * l.center, l.extents }, r, inv_rot * movement);
    if (result)
    {
        result->contact = l.rotation * result->contact;
        result->normal = l.rotation * result->normal;
    }
    return result;
}

std::optional<Intersection> intersectMoving(const Sphere& l, const Obb& r, const glm::vec3& movement)
{
    return swapAndIntersect(l, r, movement);
}
