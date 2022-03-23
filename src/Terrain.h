#ifndef CGT_TERRAIN_H
#define CGT_TERRAIN_H

#include "Component.h"
#include "SafeGl.h"
#include "Mesh.h"
#include "Transformation.h"

class Terrain : public DependentComponent<Transformation>, public Drawable
{
public:
    Terrain(const EntityKey& key, Image heightmap, Image grass, Image rock, Image snow);
    void draw(const glm::mat4& v, const glm::mat4& p) const override;
    float getAltitudeOf(const glm::vec3& pos) const;

private:
    void rebuildMesh();

    Image _raw_heightmap;
    Image _raw_grass;
    Image _raw_rock;
    Image _raw_snow;
    SafeGl::VertexArray _vao;
    SafeGl::Buffer _vertex_buffer;
    SafeGl::Buffer _element_buffer;
    SafeGl::Texture _heightmap;
    SafeGl::Texture _grass;
    SafeGl::Texture _rock;
    SafeGl::Texture _snow;
    SafeGl::Program _program;
    std::size_t _size;
    float _min_height = 0;
    float _max_height = 1;
};

#endif
