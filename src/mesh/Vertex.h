#pragma once

#include <glm/glm.hpp>

#include "asset/Material.h"

namespace mesh {
    struct Vertex final
    {
    public:
        Vertex()
        {}

        Vertex(
            const glm::vec3& pos,
            const glm::vec2& texture,
            const glm::vec3& normal,
            const glm::vec3& tangent,
            const ki::material_id materialID)
            : pos(pos),
            texture(texture),
            normal(normal),
            tangent(tangent),
            materialID(materialID)
        {
        }

        Vertex(const Vertex& b) noexcept
            : pos(b.pos),
            texture(b.texture),
            normal(b.normal),
            tangent(b.tangent),
            materialID(b.materialID)
        {
        }

        Vertex& operator=(const Vertex& b)
        {
            pos = b.pos;
            texture = b.texture;
            normal = b.normal;
            tangent = b.tangent;
            materialID = b.materialID;

            return *this;
        }

        bool operator==(const Vertex& b) const noexcept;

        bool operator!=(const Vertex& b) const noexcept;

        std::string str() const noexcept;

    public:
        glm::vec3 pos;
        glm::vec2 texture;
        glm::vec3 normal;
        glm::vec3 tangent;

        ki::material_id materialID;
    };
}
