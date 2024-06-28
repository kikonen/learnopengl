#pragma once

#include <string>

#include <glm/glm.hpp>

namespace loader {
    struct AttachmentData {
        std::string mesh;
        std::string socket;

        glm::vec3 offset{ 0.f };
        glm::vec3 rotation{ 0.f };
    };
}
