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
            glm::quat rotation,
            glm::vec3 scale,
            glm::vec3 meshScale);

        const std::string m_name;
        const std::string m_jointName;
        const glm::mat4 m_transform;

        const glm::vec3 m_offset;
        const glm::quat m_rotation;
        const glm::vec3 m_scale;
        const glm::vec3 m_meshScale;

        const glm::mat4 m_meshScaleTransform;
        const glm::mat4 m_invMeshScaleTransform;

        int16_t m_index{ -1 };
        int16_t m_jointIndex{ -1 };
    };
}
