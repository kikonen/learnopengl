#pragma once

#include <string>

#include <glm/glm.hpp>

namespace loader {
    struct SocketData {
        bool enabled{ false };

        std::string name;
        std::string joint;

        glm::vec3 offset{ 0.f };
        glm::vec3 rotation{ 0.f };
        glm::vec3 scale{ 1.f };
    };
}
