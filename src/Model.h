#ifndef CGT_MODEL_H
#define CGT_MODEL_H

#include "Component.h"
#include "SafeGl.h"
#include "Mesh.h"
#include "Transformation.h"

class Model : public DependentComponent<Transformation>, public Drawable
{
public:
    Model(const EntityKey& key, Mesh mesh);
    void draw(const glm::mat4& vp) const override;
private:
    void rebuildMesh();

    Mesh _mesh;
    SafeGl::VertexArray _vao;
    SafeGl::Buffer _vertex_buffer;
    SafeGl::Buffer _element_buffer;
    SafeGl::Texture _texture;
    SafeGl::Program _program;
};

#endif
