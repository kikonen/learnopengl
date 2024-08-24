#pragma once

#include <glm/glm.hpp>

#include "material/Material.h"

namespace mesh {
    struct Vertex final
    {
    public:
        Vertex()
        {}

        Vertex(
            const glm::vec3& pos,
            const glm::vec2& texCoord,
            const glm::vec3& normal,
            const glm::vec3& tangent)
            : pos{ pos },
            texCoord{ texCoord },
            normal{ normal },
            tangent{ tangent }
        {
        }

        Vertex(const Vertex& b) noexcept
            : pos{ b.pos },
            texCoord{ b.texCoord },
            normal{ b.normal },
            tangent{ b.tangent }
        {
        }

        Vertex& operator=(const Vertex& b)
        {
            pos = b.pos;
            texCoord = b.texCoord;
            normal = b.normal;
            tangent = b.tangent;

            return *this;
        }

        bool operator==(const Vertex& b) const noexcept;

        bool operator!=(const Vertex& b) const noexcept;

        std::string str() const noexcept;

    public:
        glm::vec3 pos;
        glm::vec2 texCoord;
        glm::vec3 normal;
        glm::vec3 tangent;
    };
}
