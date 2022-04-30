#include "Mesh.h"
#include <cmath>
#include <numbers>
#include <algorithm>

namespace
{
    constexpr unsigned vertIndex(unsigned iX, unsigned iY, unsigned nX, unsigned nY)
    {
        return iX + iY * nX;
    }

    glm::vec3 sphereVertice(float x, float y)
    {
        using std::cos, std::sin;
        float pi = std::numbers::pi_v<float>;

        float theta = x * 2 * pi;
        float phi = y * pi - pi / 2.f;
        return { cos(theta) * cos(phi),
            sin(phi),
            sin(theta) * cos(phi) };
    }
}

void setUnitSphere(MeshData& o_mesh, unsigned nX, unsigned nY)
{
    ++nX; //Need a duplicate line to allow texture wrapping
    nY += 2; //Top and bottom
    const unsigned nbVertices = nX * nY;
    o_mesh.vertices.resize(nbVertices);
    o_mesh.normals.resize(nbVertices);
    o_mesh.uvs.resize(nbVertices);
    for (unsigned iX = 0; iX < nX; ++iX)
    {
        for (unsigned iY = 0; iY < nY; ++iY)
        {
            unsigned idx = vertIndex(iX, iY, nX, nY);
            glm::vec2 uv{ float(iX) / (nX - 1), float(iY) / (nY - 1) };
            o_mesh.uvs[idx] = uv;
            o_mesh.vertices[idx] = sphereVertice(uv.x, uv.y);
            o_mesh.normals[idx] = sphereVertice(uv.x, uv.y);
        }
    }

    o_mesh.triangles.resize((nX - 1) * (nY - 1) * 2);
    auto it = begin(o_mesh.triangles);
    for (unsigned iX = 0; iX < nX - 1; ++iX)
    {
        for (unsigned iY = 0; iY < nY - 1; ++iY)
        {
            *(it++) = {vertIndex(iX, iY, nX, nY),
                vertIndex(iX, iY + 1, nX, nY),
                vertIndex(iX + 1, iY, nX, nY) };
            *(it++) = { vertIndex(iX + 1, iY, nX, nY),
                vertIndex(iX, iY + 1, nX, nY),
                vertIndex(iX + 1, iY + 1, nX, nY) };
        }
    }
}

namespace
{
    glm::vec4 lerp(glm::vec4 a, glm::vec4 b, float t)
    {
        return a * (1 - t) + b * t;
    }
    glm::vec4 toVec(Rgba x)
    {
        return { x.r, x.g, x.b, x.a };
    }
}

glm::vec4 sampleImage(const Image& img, float u, float v)
{
    using std::clamp;
    u = clamp(u, 0.f, 1.f) * (img.width - 1);
    v = clamp(v, 0.f, 1.f) * (img.height - 1);
    unsigned u_left = static_cast<unsigned>(u);
    unsigned u_right = clamp(u_left + 1, 0u, img.width-1);
    unsigned v_top = static_cast<unsigned>(v);
    unsigned v_bot = clamp(v_top + 1, 0u, img.height-1);

    glm::vec4 top_left = toVec(img.pixels[v_top * img.width + u_left]) / 255.f;
    glm::vec4 top_right = toVec(img.pixels[v_top * img.width + u_right]) / 255.f;
    glm::vec4 bot_left = toVec(img.pixels[v_bot * img.width + u_left]) / 255.f;
    glm::vec4 bot_right = toVec(img.pixels[v_bot * img.width + u_right]) / 255.f;

    return lerp(lerp(top_left, top_right, u - u_left), lerp(bot_left, bot_right, u - u_left), v - v_top);
}

SafeGl::Texture toTexture(const Image& img)
{
    SafeGl::Texture tex;

    glGenTextures(1, handleInit(tex));
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    return tex;
}

Mesh::Mesh(const MeshData& data)
{
    glGenVertexArrays(1, handleInit(_vao));
    unsigned stride = 3;
    if (!data.normals.empty()) stride += 3;
    if (!data.uvs.empty()) stride += 2;
    std::vector<float> vertices(data.vertices.size() * stride);
    auto it = std::begin(vertices);
    for (std::size_t i = 0; i < data.vertices.size(); ++i)
    {
        glm::vec3 v = data.vertices[i];
        *(it++) = v.x;
        *(it++) = v.y;
        *(it++) = v.z;
        if (!data.normals.empty())
        {
            glm::vec3 n = data.normals[i];
            *(it++) = n.x;
            *(it++) = n.y;
            *(it++) = n.z;
        }
        if (!data.uvs.empty())
        {
            glm::vec2 uv = data.uvs[i];
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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.triangles.size() * 3 * sizeof(unsigned), data.triangles.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
    if (!data.normals.empty())
    {
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(3 * sizeof(float)));
    }
    if (!data.uvs.empty())
    {
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)((data.normals.empty() ? 3 : 6) * sizeof(float)));
    }
    _size = data.triangles.size() * 3;
}

void Mesh::drawCall() const
{
    glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, _size, GL_UNSIGNED_INT, nullptr);
}
