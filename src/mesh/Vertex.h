#pragma once

#include <glm/glm.hpp>

#include "material/Material.h"

namespace mesh {
    // NOTE KI have to either store tangent as glm::vec4 to retained
    // handedness in w, or store tangent and bitangent separately
    struct Vertex final
    {
    public:
        Vertex() = default;

        Vertex(
            const glm::vec3& pos,
            const glm::vec2& texCoord,
            const glm::vec3& normal,
            const glm::vec3& tangent,
            const glm::vec3& bitangent)
            : pos{ pos },
            texCoord{ texCoord },
            normal{ normal },
            tangent{ tangent },
            bitangent{ bitangent }
        {
        }

        Vertex(const Vertex& b) noexcept = default;
        Vertex& operator=(const Vertex& b) = default;

        Vertex(Vertex&&) noexcept = default;
        Vertex& operator=(Vertex&&) noexcept = default;

        bool operator==(const Vertex& b) const noexcept;

        bool operator!=(const Vertex& b) const noexcept;

        std::string str() const noexcept;

    public:
        glm::vec3 pos;
        glm::vec2 texCoord;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec3 bitangent;
    };
}
