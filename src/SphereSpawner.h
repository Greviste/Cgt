#ifndef CGT_SPHERESPAWNER_H
#define CGT_SPHERESPAWNER_H

#include "Component.h"
#include "Transformation.h"


class SphereSpawner : public DependentComponent<Transformation>
{
public:
    using DependentComponent::DependentComponent;

    void start();
    void spawnInput(bool val);
};

#endif
 