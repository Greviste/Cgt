#ifndef CGT_LIGHT_H
#define CGT_LIGHT_H

#include "Component.h"
#include "Transformation.h"
#include "SafeGl.h"

class Light : public DependentComponent<Transformation>
{
public:
    Light(const EntityKey& key);

    LightData buildData() const;

    glm::vec3 color{1,1,1};
    float intensity = 100;
    float range = 50;
    float fov = 90;
private:
    SafeGl::FrameBuffer _fbo;
    SafeGl::Texture _tex;
};

#endif
