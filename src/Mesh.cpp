//
// Created by tk2 on 12/10/25.
//
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <memory>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Mesh.h"
MeshData Mesh::LoadModelIndexed(const std::string& inputfile) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, inputfile.c_str());

    if (!warn.empty()) std::cout << "TinyObj Warning: " << warn << std::endl;
    if (!err.empty()) std::cerr << "TinyObj Error: " << err << std::endl;
    if (!ret) exit(1);

    MeshData meshData;

    std::map<std::tuple<int, int, int>, uint32_t> uniqueVertices;

    for (const auto& shape : shapes) {
        size_t index_offset = 0;

        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            int fv = shape.mesh.num_face_vertices[f];

            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

                auto key = std::make_tuple(idx.vertex_index, idx.normal_index, idx.texcoord_index);

                if (uniqueVertices.count(key) == 0) {
                    Vertex vertex;
                    vertex.x = attrib.vertices[3 * idx.vertex_index + 0];
                    vertex.y = attrib.vertices[3 * idx.vertex_index + 1];
                    vertex.z = attrib.vertices[3 * idx.vertex_index + 2];

                    if (idx.normal_index >= 0) {
                        vertex.nx = attrib.normals[3 * idx.normal_index + 0];
                        vertex.ny = attrib.normals[3 * idx.normal_index + 1];
                        vertex.nz = attrib.normals[3 * idx.normal_index + 2];
                    }

                    if (idx.texcoord_index >= 0) {
                        vertex.u = attrib.texcoords[2 * idx.texcoord_index + 0];
                        vertex.v = attrib.texcoords[2 * idx.texcoord_index + 1];
                    }

                    meshData.vertices.push_back(vertex);

                    uint32_t newIndex = static_cast<uint32_t>(meshData.vertices.size() - 1);
                    uniqueVertices[key] = newIndex;

                    meshData.indices.push_back(newIndex);
                } else {
                    meshData.indices.push_back(uniqueVertices[key]);
                }
            }
            index_offset += fv;
        }
    }

    if (!meshData.vertices.empty()) {
        // 1. Znajdź skrajne punkty (Min/Max)
        float minX = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::lowest();
        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::lowest();
        float minZ = std::numeric_limits<float>::max();
        float maxZ = std::numeric_limits<float>::lowest();

        for (const auto& v : meshData.vertices) {
            if (v.x < minX) minX = v.x;
            if (v.x > maxX) maxX = v.x;
            if (v.y < minY) minY = v.y;
            if (v.y > maxY) maxY = v.y;
            if (v.z < minZ) minZ = v.z;
            if (v.z > maxZ) maxZ = v.z;
        }

        // 2. Oblicz środek i wymiary
        float centerX = (minX + maxX) / 2.0f;
        float centerY = (minY + maxY) / 2.0f;
        float centerZ = (minZ + maxZ) / 2.0f;

        float width = maxX - minX;
        float height = maxY - minY;
        float depth = maxZ - minZ;

        // 3. Oblicz skalę
        // Wybieramy największy wymiar, żeby zachować proporcje (aspect ratio) modelu
        float maxDim = std::max({width, height, depth});

        // Chcemy, żeby maxDim stało się równe 2.0 (od -1 do 1)
        // Zabezpieczenie przed dzieleniem przez zero dla pojedynczego punktu
        float scale = (maxDim > 0) ? (2.0f / maxDim) : 1.0f;

        // 4. Zastosuj transformację do wszystkich wierzchołków
        for (auto& v : meshData.vertices) {
            // Najpierw centrujemy (odejmujemy środek), potem skalujemy
            v.x = (v.x - centerX) * scale;
            v.y = (v.y - centerY) * scale;
            v.z = (v.z - centerZ) * scale;
        }

        std::cout << "Model znormalizowany. Skala: " << scale << " Srodek: "
                  << centerX << ", " << centerY << ", " << centerZ << std::endl;
    }

    std::cout << "Wczytano unikalnych wierzchołków: " << meshData.vertices.size() << std::endl;
    std::cout << "Liczba indeksów: " << meshData.indices.size() << std::endl;

    return meshData;
}



Mesh::Mesh() {
    const MeshData mesh = LoadModelIndexed("cat.obj");
    vbo.set_data(mesh.vertices);
    ebo.set_data(mesh.indices);
    glm::uint stride = sizeof(Vertex);

    vao.attrib_index(0).bind_attrib(
        vbo,
        offsetof(Vertex, x),
        stride,
        3,
        GL_FLOAT,
        gfx::NOT_INSTANCED
    );

    vao.attrib_index(1).bind_attrib(
        vbo,
        offsetof(Vertex, nx),
        stride,
        3,
        GL_FLOAT,
        gfx::NOT_INSTANCED
    );
    vao.attrib_index(2).bind_attrib(
        vbo,
        offsetof(Vertex, u),
        stride,
        2,
        GL_FLOAT,
        gfx::NOT_INSTANCED
    );
    vao.bind();
    ebo.bind();
    vao.unbind();
    ebo.unbind();
    program.vertex({"RenderingShaders/mesh.vs.glsl"}).fragment({"RenderingShaders/mesh.fs.glsl"}).compile();
    indicesCount = mesh.indices.size();

}

void Mesh::Draw(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& eye)
{
    program.use();
    glUniformMatrix4fv(program.uniform_loc("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(program.uniform_loc("view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniform3f(program.uniform_loc("viewPos"), eye.x, eye.y, eye.z);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = translate(modelMatrix, glm::vec3(0.0f,-1.0f,0.0f));
    glUniformMatrix4fv(program.uniform_loc("model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));

    vao.bind();

    glDrawElements(GL_TRIANGLES, this->indicesCount, GL_UNSIGNED_INT, 0);

    vao.unbind();
    program.disuse();
}
