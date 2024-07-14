#include "RigSocket.h"

#include <glm/glm.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

namespace {
    glm::mat4 calcTransform(
        glm::vec3 offset,
        glm::quat rotation,
        glm::vec3 scale)
    {
        return glm::translate(glm::mat4{ 1.f }, offset) *
            glm::toMat4(rotation) *
            glm::scale(glm::mat4{ 1.f }, scale);
    }

    glm::mat4 calcMeshScaleTransform(
        glm::vec3 meshScale)
    {
        return glm::scale(glm::mat4{ 1.f }, meshScale);
    }
}

namespace animation
{
    RigSocket::RigSocket(
        const std::string& name,
        const std::string& jointName,
        glm::vec3 offset,
        glm::quat rotation,
        glm::vec3 scale,
        glm::vec3 meshScale)
        : m_name{ name },
        m_jointName{ jointName },
        m_offset{ offset },
        m_rotation{ rotation },
        m_scale{ scale },
        m_meshScale{ meshScale },
        m_transform{ calcTransform(offset, rotation, scale) },
        m_meshScaleTransform{ calcMeshScaleTransform(meshScale) },
        m_invMeshScaleTransform{ glm::inverse(m_meshScaleTransform) }
    {}
}
