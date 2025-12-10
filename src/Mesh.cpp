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

std::vector<Vertex> Mesh::LoadModel(const std::string& inputfile) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;

        std::string warn;
        std::string err;

        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, inputfile.c_str());

        if (!warn.empty()) {
            std::cout << "TinyObj Warning: " << warn << std::endl;
        }
        if (!err.empty()) {
            std::cerr << "TinyObj Error: " << err << std::endl;
        }
        if (!ret) {
            exit(1);
        }

        std::vector<Vertex> verticesForOpenGL;

        for (size_t s = 0; s < shapes.size(); s++) {

            size_t index_offset = 0;

            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {

                int fv = shapes[s].mesh.num_face_vertices[f];

                for (size_t v = 0; v < fv; v++) {
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

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
                    verticesForOpenGL.push_back(vertex);
                }

                index_offset += fv;
            }
        }

        std::cout << "Wczytano wierzchołków: " << verticesForOpenGL.size() << std::endl;
        return verticesForOpenGL;
    }

Mesh::Mesh() {
    const std::vector<Vertex> data = LoadModel("cat.obj");
    vbo.set_data(data);
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
    program.vertex({"RenderingShaders/mesh.vs.glsl"}).fragment({"RenderingShaders/mesh.fs.glsl"}).compile();
    vertexCount = data.size();
}

void Mesh::Draw(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& eye)
{
    program.use();
    glUniformMatrix4fv(program.uniform_loc("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(program.uniform_loc("view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniform3f(program.uniform_loc("viewPos"), eye.x, eye.y, eye.z);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -1.0f, 0.0f));

    // 2. Skalowanie (Scale) - TO JEST KLUCZOWE
    // Zmniejszamy model 100-krotnie (0.01f) lub 10-krotnie (0.1f)
    // Eksperymentuj z tą wartością, aż model zmieści się na ekranie.
    float scaleFactor = 0.002f;
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scaleFactor));
    glUniformMatrix4fv(program.uniform_loc("model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));

    vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, this->vertexCount);
    vao.unbind();
    program.disuse();
}
