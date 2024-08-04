#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "mesh/PrimitiveType.h"

namespace loader {
    struct VertexData {
        bool valid{ false };

        mesh::PrimitiveType type{ mesh::PrimitiveType::none };

        std::string name;
        std::string alias;

        glm::vec3 size{ 1.f };
        float inner_radius = 0.f;
        float radius = 1.f;
        float length{ 0.5f };
        int slices{ 32 };
        int segments{ 4 };
        int rings{ 8 };

        std::vector<glm::vec3> vertices;
        std::vector<int> indeces;
    };
}
