#include <glm/glm.hpp>
#include <vector>
#include <limits>

class Mesh {
public:
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> faces; // Assumes triangles: [v0, v1, v2, v0, v1, v2, ...]

    void normalize() {
        if (vertices.empty()) return;


        glm::vec3 min = vertices[0];
        glm::vec3 max = vertices[0];

        for (const auto& v : vertices) {
            min.x = std::min(min.x, v.x);
            min.y = std::min(min.y, v.y);
            min.z = std::min(min.z, v.z);

            max.x = std::max(max.x, v.x);
            max.y = std::max(max.y, v.y);
            max.z = std::max(max.z, v.z);
        }

        // Calculate range
        glm::vec3 range = max - min;

        // Find maximum range to maintain aspect ratio
        float maxRange = std::max(std::max(range.x, range.y), range.z);

        // Avoid division by zero
        if (maxRange < 0.0000001f) return;

        // Normalize vertices to [0, 1]
        for (auto& v : vertices) {
            v = (v - min) / maxRange;
        }
    }

    bool isPointInside(const glm::vec3& point) const {
        // Ray direction (arbitrary, we use +X axis)
        glm::vec3 rayDir(1.0f, 0.0f, 0.0f);

        int intersectionCount = 0;

        // Check each triangle
        for (size_t i = 0; i < faces.size(); i += 3) {
            glm::vec3 v0 = vertices[faces[i]];
            glm::vec3 v1 = vertices[faces[i + 1]];
            glm::vec3 v2 = vertices[faces[i + 2]];

            if (rayIntersectsTriangle(point, rayDir, v0, v1, v2)) {
                intersectionCount++;
            }
        }

        // Odd = inside, Even = outside
        return (intersectionCount % 2) == 1;
    }

private:
    bool rayIntersectsTriangle(const glm::vec3& rayOrigin,
                               const glm::vec3& rayDir,
                               const glm::vec3& v0,
                               const glm::vec3& v1,
                               const glm::vec3& v2) const {
        // Möller–Trumbore intersection algorithm
        const float EPSILON = 0.0000001f;

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 h = glm::cross(rayDir, edge2);
        float a = glm::dot(edge1, h);

        // Ray is parallel to triangle
        if (a > -EPSILON && a < EPSILON)
            return false;

        float f = 1.0f / a;
        glm::vec3 s = rayOrigin - v0;
        float u = f * glm::dot(s, h);

        if (u < 0.0f || u > 1.0f)
            return false;

        glm::vec3 q = glm::cross(s, edge1);
        float v = f * glm::dot(rayDir, q);

        if (v < 0.0f || u + v > 1.0f)
            return false;

        // Compute t to find intersection point
        float t = f * glm::dot(edge2, q);

        // Ray intersection (t > EPSILON ensures we only count forward intersections)
        return t > EPSILON;
    }
};