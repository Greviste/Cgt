#include "SimpleMovement.h"
#include "Input.h"
#include <cmath>
#include <numbers>

void SimpleMovement::update(Seconds delta)
{
    glm::quat rot = get<Transformation>().rotation();
    glm::vec3 pos = get<Transformation>().translation();

    float dist = _speed * delta.count();
    if (Inputs::instance().forward.value())
        pos += rot * glm::vec3{ 0,0,-1 } * dist;
    if (Inputs::instance().backward.value())
        pos += rot * glm::vec3{ 0,0,1 } * dist;
    if (Inputs::instance().right.value())
        pos += rot * glm::vec3{ 1,0,0 } * dist;
    if (Inputs::instance().left.value())
        pos += rot * glm::vec3{ -1,0,0 } * dist;

    glm::vec2 look = Inputs::instance().look.value();
    _horizontal_angle += look.x * _rotation_ratio;
    _vertical_angle = std::clamp(_vertical_angle + look.y * _rotation_ratio, -1.5f, 1.5f);
    get<Transformation>().rotation(glm::vec3{ _vertical_angle, _horizontal_angle, 0 });

    if (_terrain)
    {
        pos.y += _fixed_height - _terrain->getAltitudeOf(pos);
    }

    get<Transformation>().translation(pos);
}

void SimpleMovement::terrain(Terrain* t)
{
    _terrain = t;
}

void SimpleMovement::terrain(Terrain& t)
{
    _terrain = t;
}

Terrain* SimpleMovement::terrain() const
{
    return _terrain.ptr();
}
