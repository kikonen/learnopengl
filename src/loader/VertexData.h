#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "mesh/PrimitiveType.h"

namespace loader {
    struct VertexData {
        bool valid{ false };

        mesh::PrimitiveType type{ mesh::PrimitiveType::none };

        std::vector<glm::vec3> vertices;
        std::vector<int> indeces;
    };
}
