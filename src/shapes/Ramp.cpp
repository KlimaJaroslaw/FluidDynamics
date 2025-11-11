// src/shapes/Ramp.cpp
#include "Ramp.hpp"
#include <cmath>
#include <iostream>

#include "Mesh.hpp"

std::vector<glm::vec3> Ramp::to_grid_impl(float scaleX, float scaleY, float scaleZ, int grid_resolution) const {
    std::vector<glm::vec3> points;
    Mesh mesh = Mesh();

    mesh.vertices = {
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 1.0f}
    };
    mesh.faces = {
        0,1,4,0,5,4,0,3,5,3,2,5,1,2,5,1,4,5,0,1,2,0,3,2
    };
    mesh.normalize();

    for (float x = 0.0f; x <= 1.0f; x += 1/float(grid_resolution)) {
        for (float y = 0.0f; y <= 1.0f; y += 1/float(grid_resolution)) {
            for (float z = 0.0f; z <= 1.0f; z += 1/float(grid_resolution)) {
                bool isIn = mesh.isPointInside(glm::vec3(x, y, z));
                if (isIn) {
                    points.emplace_back(
                        x * scaleX,
                        y * scaleY,
                        z * scaleZ
                    );
                }
            }
        }
    }
    return points;
}
