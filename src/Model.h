#ifndef CGT_MODEL_H
#define CGT_MODEL_H

#include "Component.h"
#include "SafeGl.h"
#include "Mesh.h"
#include "Transformation.h"

class Model : public DependentComponent<Transformation>, public Drawable
{
public:
    Model(const EntityKey& key, const Image& img, const MeshData& mesh);
    void draw(const glm::mat4& vp) const override;
private:
    Mesh _mesh;
    SafeGl::Program _program;
    SafeGl::Texture _texture;
};

#endif
