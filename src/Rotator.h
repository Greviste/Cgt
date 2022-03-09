#ifndef CGT_ROTATOR_H
#define CGT_ROTATOR_H

#include "Component.h"
#include "Transformation.h"
#include <chrono>


class Rotator : public DependentComponent<Transformation>
{
public:
    Rotator(const EntityKey& key, float rotation_rate);
    void update(std::chrono::duration<float> delta);
private:
    float _rotation_rate;
    float _current_rotation = 0;
};

#endif
