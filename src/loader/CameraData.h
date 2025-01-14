#pragma once

#include <array>

#include <glm/glm.hpp>

#include "CameraType.h"

namespace loader {
    struct CameraData {
        CameraType type{ CameraType::fps };

        bool enabled{ false };

        bool isDefault{ false };

        bool orthogonal{ false };
        std::array<float, 4> viewport{ 0.f };

        float fov{ 45.f };

        glm::vec3 front{ 0.f, 0.f, -1.f };
        glm::vec3 up{ 0.f, 1.f, 0.f };

        glm::vec3 offset{ 40.f, 0.f, 0.f };

        float pitchSpeed{ 0.f };
        float yawSpeed{ 0.f };

        // [horiz_offset, vert_offset, target_dist
        glm::vec3 distance{ 0.f };

        // 64
        float springConstant{ 1024.f };

        std::vector<glm::vec3> path;
        float speed{ 0.f };
    };
}
