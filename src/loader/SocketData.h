#pragma once

#include <glm/glm.hpp>

namespace loader {
    struct SocketData {
        // Joint name
        std::string name;

        glm::vec3 offset{ 0.f };
        glm::vec3 rotation{ 0.f };
    };
}
