#pragma once

#include <glm/glm.hpp>

#include "Material.h"

class Vertex final
{
public:
    Vertex(
        const glm::vec3& pos,
        const glm::vec2& texture,
        const glm::vec3& normal,
        const glm::vec3& tangent,
        const int materialID);

    bool operator==(const Vertex& b) const;
    bool operator!=(const Vertex& b) const;
public:
    const glm::vec3 pos;
    const glm::vec2 texture;
    const glm::vec3 normal;
    const glm::vec3 tangent;

    const int materialID;

    size_t index = 0;
};
