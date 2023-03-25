#pragma once

#include <glm/glm.hpp>

#include "Material.h"

struct Vertex final
{
public:
    Vertex(
        const glm::vec3& pos,
        const glm::vec2& texture,
        const glm::vec3& normal,
        const glm::vec3& tangent,
        const int materialID)
        : pos(pos),
        texture(texture),
        normal(normal),
        tangent(tangent),
        materialID(materialID)
    {
    }

    inline bool operator==(const Vertex& b) const noexcept
    {
        return pos == b.pos &&
            texture == b.texture &&
            normal == b.normal &&
            tangent == b.tangent &&
            materialID == b.materialID;
    }

    inline bool operator!=(const Vertex& b) const noexcept
    {
        return !(*this == b);
    }

    const std::string str() const noexcept;

public:
    const glm::vec3 pos;
    const glm::vec2 texture;
    const glm::vec3 normal;
    const glm::vec3 tangent;

    const int materialID;
};
