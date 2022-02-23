#include "Model.h"

using namespace SafeGl;

Model::Model(const EntityKey& key, Mesh mesh)
    : DependentComponent(key), _mesh(std::move(mesh)), _program(SafeGl::loadAndCompileProgram("res/vertex_shader.glsl", "res/fragment_shader.glsl"))
{
    glGenVertexArrays(1, handleInit(_vao));
    rebuildMesh();
}

void Model::rebuildMesh()
{
    unsigned stride = 3;
    if (!_mesh.normals.empty()) stride += 3;
    if (!_mesh.uvs.empty()) stride += 2;
    std::vector<float> vertices(_mesh.vertices.size() * stride);
    auto it = std::begin(vertices);
    for (std::size_t i = 0; i < _mesh.vertices.size(); ++i)
    {
        glm::vec3 v = _mesh.vertices[i];
        *(it++) = v.x;
        *(it++) = v.y;
        *(it++) = v.z;
        if (!_mesh.normals.empty())
        {
            glm::vec3 n = _mesh.normals[i];
            *(it++) = n.x;
            *(it++) = n.y;
            *(it++) = n.z;
        }
        if (!_mesh.uvs.empty())
        {
            glm::vec2 uv = _mesh.uvs[i];
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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _mesh.triangles.size() * 3 * sizeof(unsigned), _mesh.triangles.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
    if (!_mesh.normals.empty())
    {
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(3*sizeof(float)));
    }
    if (!_mesh.uvs.empty())
    {
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)((_mesh.normals.empty() ? 3 : 6) * sizeof(float)));
    }
    glGenTextures(1, handleInit(_texture));
    glBindTexture(GL_TEXTURE_2D, _texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _mesh.texture.width, _mesh.texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _mesh.texture.pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void Model::draw(glm::mat4 vp) const
{
    glBindVertexArray(_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glUseProgram(_program);
    vp *= get<Transformation>().matrix();
    glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);
    glDrawElements(GL_TRIANGLES, _mesh.triangles.size() * 3, GL_UNSIGNED_INT, nullptr);
}
