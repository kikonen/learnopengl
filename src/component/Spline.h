#pragma once

#include <vector>

#include <glm/glm.hpp>

struct Spline {
    std::vector<glm::vec3> m_controlPoints;

    glm::vec3 calculatePosition(size_t startIndex, float t);
};
