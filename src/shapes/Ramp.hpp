

#ifndef RAMP_HPP
#define RAMP_HPP
#include "Shapes.hpp"


class Ramp : public IShape {
public:
    std::vector<glm::vec3> to_grid_impl(
        float scaleX,
        float scaleY,
        float scaleZ,
        int grid_resolution
    ) const override;
};



#endif //RAMP_HPP
