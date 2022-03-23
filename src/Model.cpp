#include "Model.h"

using namespace SafeGl;

Model::Model(const EntityKey& key, const Image& img, const MeshData& mesh)
    :DependentComponent(key), _mesh(mesh), _program(SafeGl::loadAndCompileProgram("res/vertex_shader.glsl", "res/fragment_shader.glsl")), _texture(toTexture(img))
{
}

void Model::draw(const glm::mat4& vp) const
{
    glUseProgram(_program);
    glm::mat4 mvp = vp * get<Transformation>().matrix();
    glUniformMatrix4fv(0, 1, GL_FALSE, &mvp[0][0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _texture);
    _mesh.drawCall();
}
