#pragma once

#include <stdint.h>

#include <glm/glm.hpp>

enum class PivotAlignment : uint8_t {
    origin,
    middle,
    top,
    bottom,
    left,
    right
};

namespace mesh {
    class MeshType;
}

struct PivotPoint {
    glm::vec3 m_offset{ 0.f };

    PivotAlignment m_alignment[3]{
        PivotAlignment::origin,
        PivotAlignment::origin,
        PivotAlignment::origin
    };

    glm::vec3 resolve(mesh::MeshType* type) const;
};

