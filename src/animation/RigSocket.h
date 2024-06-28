#pragma once

#include <string>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace animation
{
    struct RigSocket
    {
        RigSocket(
            const std::string& name,
            const std::string& jointName,
            glm::vec3 offset,
            glm::quat rotation);

        const std::string m_name;
        const std::string m_jointName;
        const glm::mat4 m_transform{ 1.f };

        int16_t m_jointIndex{ -1 };
        int16_t m_socketIndex{ -1 };
    };
}
