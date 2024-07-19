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
            float scale,
            glm::vec3 meshScale);

        void updateTransforms();

        const std::string m_name;
        const std::string m_jointName;

        glm::mat4 m_transform;

        glm::vec3 m_offset;
        glm::quat m_rotation;
        float m_scale;
        glm::vec3 m_meshScale;

        glm::mat4 m_meshScaleTransform;
        glm::mat4 m_invMeshScaleTransform;

        int16_t m_index{ -1 };
        int16_t m_jointIndex{ -1 };
    };
}
