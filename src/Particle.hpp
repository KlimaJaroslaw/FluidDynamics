#pragma once
#include <glm/glm.hpp>

const int INACTIVE = 0;
const int ACTIVE = 1;

struct Particle {
    alignas(16) glm::vec4 color;
    alignas(16) glm::vec3 pos;
    alignas(16) glm::vec3 vel;
    alignas(4) int type = 0;

    Particle(glm::vec3 pos, glm::vec3 vel, glm::vec4 color) : color(color), pos(pos), vel(vel) {}
};
