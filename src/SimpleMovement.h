#ifndef CGT_SIMPLEMOVEMENT_H
#define CGT_SIMPLEMOVEMENT_H

#include "Component.h"
#include "Transformation.h"
#include "Terrain.h"


class SimpleMovement : public DependentComponent<Transformation>
{
public:
    using DependentComponent::DependentComponent;

    void update(Seconds delta);
    void terrain(Terrain* t);
    void terrain(Terrain& t);
    Terrain* terrain() const;

private:
    float _fixed_height = 0.1;
    float _speed = 1;
    WeakRef<Terrain> _terrain;
};

#endif
