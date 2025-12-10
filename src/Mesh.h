//
// Created by tk2 on 12/10/25.
//

#ifndef GL_PIC_FLUID_MESH_H
#define GL_PIC_FLUID_MESH_H
#include <bits/basic_string.h>
#include <bits/stl_vector.h>

#include "gfx/object.hpp"
#include "gfx/program.hpp"
#include "glm/fwd.hpp"

// Przykładowa struktura wierzchołka dla Twojego silnika
struct alignas(16) Vertex {
    float x, y, z;
    float nx, ny, nz; // Normals
    float u, v;       // TexCoords
};

class Mesh
{
public:
    gfx::Buffer vbo{GL_ARRAY_BUFFER};
    gfx::VAO vao;
    gfx::Program program;
    int vertexCount;
    std::vector<Vertex> LoadModel(const std::string& inputfile);
    Mesh();
    void Draw(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& eye);
};


#endif //GL_PIC_FLUID_MESH_H