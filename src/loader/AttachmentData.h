#pragma once

#include <string>

#include <glm/glm.hpp>

namespace loader {
    struct AttachmentData {
        bool enabled{ false };

        std::string name;
        std::string socket;

        glm::vec3 offset{ 0.f };
        glm::vec3 rotation{ 0.f };
        glm::vec3 scale{ 1.f };
    };
}
