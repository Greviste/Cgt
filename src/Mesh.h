#ifndef CGT_MESH_H
#define CGT_MESH_H

#include <vector>
#include <glm/glm.hpp>
#include <SafeGl.h>

struct Rgba
{
    unsigned char r, g, b, a;
};

struct Image
{
    std::vector<Rgba> pixels;
    unsigned height, width;
};

struct Triangle
{
    unsigned a, b, c;
};

struct MeshData
{
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<Triangle> triangles;
    Image texture;
};

void setUnitSphere(MeshData& o_mesh, unsigned nX = 20, unsigned nY = 20);
glm::vec4 sampleImage(const Image& img, float u, float v);
SafeGl::Texture toTexture(const Image& img);

#endif
