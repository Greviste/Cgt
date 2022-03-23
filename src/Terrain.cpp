#include "Terrain.h"

using namespace SafeGl;

namespace
{
    std::tuple<std::vector<glm::vec2>, std::vector<unsigned short>> generatePlane(size_t n)
    {
        std::vector<glm::vec2> vertices(n * n);
        std::vector<unsigned short> indices((n - 1) * (n - 1) * 2 * 3);
        auto next_vertice = begin(vertices);
        auto next_indice = begin(indices);

        for (size_t i = 0; i < n; ++i)
        {
            float i_coord = (float)i / (n - 1) * 2 - 1;
            for (size_t j = 0; j < n; ++j)
            {
                float j_coord = (float)j / (n - 1) * 2 - 1;
                *(next_vertice++) = { i_coord,j_coord };
            }
        }

        for (size_t i = 0; i < n - 1; ++i)
        {
            for (size_t j = 0; j < n - 1; ++j)
            {
                *(next_indice++) = i + j * n;
                *(next_indice++) = i + (j + 1) * n;
                *(next_indice++) = i + 1 + j * n;

                *(next_indice++) = i + (j + 1) * n;
                *(next_indice++) = i + 1 + (j + 1) * n;
                *(next_indice++) = i + 1 + j * n;
            }
        }

        return { vertices, indices };
    }
}

Terrain::Terrain(const EntityKey& key, Image heightmap, Image grass, Image rock, Image snow)
    : DependentComponent(key), _raw_heightmap(std::move(heightmap)), _raw_grass(std::move(grass)), _raw_rock(std::move(rock)), _raw_snow(std::move(snow)), _program(SafeGl::loadAndCompileProgram("res/terrain_vertex_shader.glsl", "res/terrain_fragment_shader.glsl"))
{
    glGenVertexArrays(1, handleInit(_vao));
    rebuildMesh();
}

void Terrain::rebuildMesh()
{
    auto [vertices, indices] = generatePlane(20);
    //VBOs can be deleted when bound to a VAO
    glBindVertexArray(_vao);
    glGenBuffers(1, handleInit(_vertex_buffer));
    glBindBuffer(GL_ARRAY_BUFFER, _vertex_buffer);
    glGenBuffers(1, handleInit(_element_buffer));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _element_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) * 2, vertices.data(), GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), indices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    _heightmap = toTexture(_raw_heightmap);
    _grass = toTexture(_raw_grass);
    _rock = toTexture(_raw_rock);
    _snow = toTexture(_raw_snow);

    _size = indices.size();
}

void Terrain::draw(const glm::mat4& vp) const
{
    glBindVertexArray(_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _heightmap);
    glActiveTexture(GL_TEXTURE0+1);
    glBindTexture(GL_TEXTURE_2D, _grass);
    glActiveTexture(GL_TEXTURE0+2);
    glBindTexture(GL_TEXTURE_2D, _rock);
    glActiveTexture(GL_TEXTURE0+3);
    glBindTexture(GL_TEXTURE_2D, _snow);
    glUseProgram(_program);
    glm::mat4 mvp = vp * get<Transformation>().matrix();
    glUniformMatrix4fv(0, 1, GL_FALSE, &mvp[0][0]);
    glUniform1f(1, _min_height);
    glUniform1f(2, _max_height);
    glDrawElements(GL_TRIANGLES, _size, GL_UNSIGNED_SHORT, nullptr);
}

float Terrain::getAltitudeOf(const glm::vec3& pos) const
{
    glm::vec4 relative = get<Transformation>().invMatrix() * glm::vec4(pos, 1);
    float u = (relative.x + 1) / 2;
    float v = (relative.z + 1) / 2;
    float raw_alt = sampleImage(_raw_heightmap, u, v).x * (_max_height - _min_height) + _min_height;

    relative.y -= raw_alt;

    return (get<Transformation>().matrix() * relative).y;
}
