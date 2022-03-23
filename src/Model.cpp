#include "Model.h"

using namespace SafeGl;

Model::Model(const EntityKey& key, MeshData mesh)
    : DependentComponent(key), _mesh_data(std::move(mesh)), _program(SafeGl::loadAndCompileProgram("res/vertex_shader.glsl", "res/fragment_shader.glsl"))
{
    glGenVertexArrays(1, handleInit(_vao));
    rebuildMesh();
}

void Model::rebuildMesh()
{
    unsigned stride = 3;
    if (!_mesh_data.normals.empty()) stride += 3;
    if (!_mesh_data.uvs.empty()) stride += 2;
    std::vector<float> vertices(_mesh_data.vertices.size() * stride);
    auto it = std::begin(vertices);
    for (std::size_t i = 0; i < _mesh_data.vertices.size(); ++i)
    {
        glm::vec3 v = _mesh_data.vertices[i];
        *(it++) = v.x;
        *(it++) = v.y;
        *(it++) = v.z;
        if (!_mesh_data.normals.empty())
        {
            glm::vec3 n = _mesh_data.normals[i];
            *(it++) = n.x;
            *(it++) = n.y;
            *(it++) = n.z;
        }
        if (!_mesh_data.uvs.empty())
        {
            glm::vec2 uv = _mesh_data.uvs[i];
            *(it++) = uv.x;
            *(it++) = uv.y;
        }
    }

    //VBOs can be deleted when bound to a VAO
    glBindVertexArray(_vao);
    glGenBuffers(1, handleInit(_vertex_buffer));
    glBindBuffer(GL_ARRAY_BUFFER, _vertex_buffer);
    glGenBuffers(1, handleInit(_element_buffer));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _element_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _mesh_data.triangles.size() * 3 * sizeof(unsigned), _mesh_data.triangles.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
    if (!_mesh_data.normals.empty())
    {
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(3*sizeof(float)));
    }
    if (!_mesh_data.uvs.empty())
    {
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)((_mesh_data.normals.empty() ? 3 : 6) * sizeof(float)));
    }
    _texture = toTexture(_mesh_data.texture);
}

void Model::draw(const glm::mat4& vp) const
{
    glBindVertexArray(_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glUseProgram(_program);
    glm::mat4 mvp = vp * get<Transformation>().matrix();
    glUniformMatrix4fv(0, 1, GL_FALSE, &mvp[0][0]);
    glDrawElements(GL_TRIANGLES, _mesh_data.triangles.size() * 3, GL_UNSIGNED_INT, nullptr);
}
