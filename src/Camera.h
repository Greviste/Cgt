#ifndef CGT_CAMERA_H
#define CGT_CAMERA_H

#include "Component.h"
#include "Transformation.h"

class Camera : public DependentComponent<Transformation>
{
public:
    using DependentComponent::DependentComponent;

    void start();

    glm::mat4 projectionMatrix() const;

    static Camera* main();
private:
    static WeakRef<Camera> _main;
};

#endif
