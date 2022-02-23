#include "Mesh.h"
#include <cmath>
#include <numbers>

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

void setUnitSphere(Mesh& o_mesh, unsigned nX, unsigned nY)
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
                vertIndex(iX + 1, iY, nX, nY),
                vertIndex(iX, iY + 1, nX, nY) };
            *(it++) = { vertIndex(iX, iY + 1, nX, nY),
                vertIndex(iX + 1, iY, nX, nY),
                vertIndex(iX + 1, iY + 1, nX, nY) };
        }
    }
}
