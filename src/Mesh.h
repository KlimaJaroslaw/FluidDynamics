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

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

class Mesh
{
public:
    gfx::Buffer vbo{GL_ARRAY_BUFFER};
    gfx::Buffer ebo{GL_ELEMENT_ARRAY_BUFFER};
    gfx::VAO vao;
    gfx::Program program;
    int indicesCount;
    std::vector<Vertex> LoadModel(const std::string& inputfile);
    MeshData LoadModelIndexed(const std::string& inputfile);
    void CreateSDF();
    Mesh();
    void Draw(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& eye);
};


#endif //GL_PIC_FLUID_MESH_H