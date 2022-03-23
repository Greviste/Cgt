#include "Model.h"
#include <glm/gtx/norm.hpp>

using namespace SafeGl;

Model::Model(const EntityKey& key, const Image& img, const MeshData& mesh)
    :DependentComponent(key), _mesh(mesh), _program(SafeGl::loadAndCompileProgram("res/vertex_shader.glsl", "res/fragment_shader.glsl")), _texture(toTexture(img))
{
}

void Model::draw(const glm::mat4& v, const glm::mat4& p) const
{
    glm::mat4 mv = v * get<Transformation>().matrix();
    glm::mat4 mvp = p * mv;
    float sqr_dist = length2(glm::vec3(mv * glm::vec4(0,0,0,1)));

    glUseProgram(_program);
    glUniformMatrix4fv(0, 1, GL_FALSE, &mvp[0][0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _texture);
    selectLod(sqr_dist).drawCall();
}

void Model::addLod(const MeshData& data, float after_dist)
{
    if (!_lods.empty()) after_dist += _lods.back().second;
    _lods.emplace_back(data, after_dist);
}

const Mesh& Model::selectLod(float sqr_dist) const
{
    for (auto it = _lods.rbegin(), l = _lods.rend(); it != l; ++it)
    {
        auto& [lod, dist] = *it;
        if (sqr_dist >= dist * dist) return lod;
    }
    return _mesh;
}
