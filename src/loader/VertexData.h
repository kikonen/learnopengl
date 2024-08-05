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

        glm::vec3 size{ 0.5f };
        float inner_radius{ 0.f };
        float radius{ 0.5f };
        float length{ 0.5f };
        int p{ 0 };
        int q{ 0 };
        int slices{ 32 };
        glm::ivec3 segments{ 4 };
        int rings{ 8 };

        glm::vec3 origin{ 0.f, 0.f, 0.f };
        glm::vec3 dir{ 0.f, 0.f, -1.f };

        bool has_size{ false };
        bool has_inner_radius{ false };
        bool has_radius{ false };
        bool has_length{ false };
        bool has_p{ false };
        bool has_q{ false };
        bool has_slices{ false };
        bool has_segments{ false };
        bool has_rings{ false };

        bool has_origin{ false };
        bool has_dir{ false };

        std::vector<glm::vec3> bezier_d0;
        std::vector<glm::vec3> bezier_d1;

        std::vector<glm::vec3> vertices;
        std::vector<int> indeces;
    };
}
