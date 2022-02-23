#ifndef CGT_MODEL_H
#define CGT_MODEL_H

#include "Component.h"
#include "SafeGl.h"
#include "Mesh.h"
#include "Transformation.h"

class Model : public DependentComponent<Transformation>
{
public:
    Model(const EntityKey& key, Mesh mesh);
    void draw(glm::mat4 vp) const;
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
