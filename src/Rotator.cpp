#include "Rotator.h"
#include <cmath>
#include <numbers>

Rotator::Rotator(const EntityKey& key, float rotation_rate)
    : DependentComponent(key), _rotation_rate(rotation_rate)
{
}

void Rotator::update(Seconds delta)
{
    _current_rotation = std::fmod(_current_rotation + _rotation_rate * delta.count(), std::numbers::pi_v<float> * 2);
    get<Transformation>().rotation(glm::vec3{ 0, _current_rotation, 0 });
}
