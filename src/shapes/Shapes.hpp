// src/shapes/Shapes.hpp
#ifndef SHAPES_HPP
#define SHAPES_HPP

#include <vector>
#include <glm/glm.hpp>


class IShape {
public:
    // Czysta metoda wirtualna bez wartości domyślnych
    virtual std::vector<glm::vec3> to_grid_impl(
        float scaleX,
        float scaleY,
        float scaleZ,
        int grid_resolution
    ) const = 0;

    // Metoda pomocnicza z wartościami domyślnymi (nie wirtualna)
    std::vector<glm::vec3> to_grid(
        float scaleX = 1.0f,
        float scaleY = 1.0f,
        float scaleZ = 1.0f,
        int grid_resolution = 12 //Number of cells along the longest axis
    ) const {
        return to_grid_impl(scaleX, scaleY, scaleZ, grid_resolution);
    }

    virtual ~IShape() = default;
};

#endif // SHAPES_HPP