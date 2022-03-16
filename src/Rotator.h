#ifndef CGT_ROTATOR_H
#define CGT_ROTATOR_H

#include "Component.h"
#include "Transformation.h"
#include <chrono>


class Rotator : public DependentComponent<Transformation>, public Updateable
{
public:
    Rotator(const EntityKey& key, float rotation_rate);
    void update(Seconds delta) override;
private:
    float _rotation_rate;
    float _current_rotation = 0;
};

#endif
