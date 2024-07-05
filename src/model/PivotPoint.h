#pragma once

#include <stdint.h>

#include <glm/glm.hpp>

enum class PivotAlignment : uint8_t {
    zero,
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
    PivotAlignment alignment[3]{
        PivotAlignment::zero,
        PivotAlignment::zero,
        PivotAlignment::zero
    };

    glm::vec3 resolve(mesh::MeshType* type) const;
};

