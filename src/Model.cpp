#include "Model.h"
#include <glm/gtx/norm.hpp>
#include <algorithm>

using namespace SafeGl;

Model::Model(const EntityKey& key, const Image& img, const MeshData& mesh)
    :DependentComponent(key), _mesh(mesh), _program(SafeGl::loadAndCompileProgram("res/vertex_shader.glsl", "res/fragment_shader.glsl")), _texture(toTexture(img))
{
}

void Model::draw(const glm::mat4& v, const glm::mat4& p) const
{
    glm::mat4 m = get<Transformation>().matrix();
    glm::mat4 inv_mv = get<Transformation>().invMatrix() * inverse(v);
    float sqr_dist = length2(glm::vec3(v * m * glm::vec4(0, 0, 0, 1)));

    glUseProgram(_program);
    glUniformMatrix4fv(0, 1, GL_FALSE, &m[0][0]);
    glUniformMatrix4fv(1, 1, GL_FALSE, &v[0][0]);
    glUniformMatrix4fv(2, 1, GL_FALSE, &p[0][0]);
    glUniformMatrix4fv(3, 1, GL_TRUE, &inv_mv[0][0]);

    auto& lights = world().lightData();
    int i;
    for (i = 0; i < std::min(lights.size(), std::size_t{ 16 }); ++i)
    {
        int offset = 10 + i * 6;
        glUniform3fv(offset, 1, &lights[i].pos[0]);
        glUniform3fv(offset + 1, 1, &lights[i].color[0]);
        glUniform1f(offset + 2, lights[i].intensity);
        glUniformMatrix4fv(offset + 3, 1, GL_FALSE, &lights[i].vp[0][0]);
        glActiveTexture(GL_TEXTURE4 + i);
        glBindTexture(GL_TEXTURE_2D, lights[i].shadowMap);
        glUniform1i(offset + 4, 4 + i);
        glUniform1i(offset + 5, 1);
    }
    for (; i < 16; ++i)
    {
        int offset = 10 + i * 6;
        glUniform1i(offset + 5, 0);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _texture);
    selectLod(sqr_dist).drawCall();
}

void Model::drawGeometry(const glm::mat4& v, const glm::mat4& p) const
{
    glm::mat4 mv = v * get<Transformation>().matrix();
    float sqr_dist = length2(glm::vec3(mv * glm::vec4(0, 0, 0, 1)));
    glm::mat4 mvp = p * mv;

    glUseProgram(geometryProgram());
    glUniformMatrix4fv(0, 1, GL_FALSE, &mvp[0][0]);
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

const Program& Model::geometryProgram()
{
    static Program p(SafeGl::loadAndCompileProgram("res/geometry_vertex_shader.glsl", "res/geometry_fragment_shader.glsl"));

    return p;
}
