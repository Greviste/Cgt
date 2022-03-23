#ifndef CGT_MODEL_H
#define CGT_MODEL_H

#include "Component.h"
#include "SafeGl.h"
#include "Mesh.h"
#include "Transformation.h"
#include <vector>
#include <utility>

class Model : public DependentComponent<Transformation>, public Drawable
{
public:
    Model(const EntityKey& key, const Image& img, const MeshData& mesh);
    void draw(const glm::mat4& v, const glm::mat4& p) const override;
    void addLod(const MeshData& data, float after_dist);

private:
    const Mesh& selectLod(float sqr_dist) const;

    Mesh _mesh;
    std::vector<std::pair<Mesh, float>> _lods;
    SafeGl::Program _program;
    SafeGl::Texture _texture;
};

#endif
