#include "RigSocket.h"

#include <glm/glm.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

namespace {
    glm::mat4 calculateLocalTransform(
        const util::Transform& offset,
        glm::vec3 meshScale)
    {
        util::Transform local = offset;

        //auto meshScaleTransform = glm::inverse(glm::scale(glm::mat4{ 1.f }, meshScale));
        auto meshScaleTransform = glm::scale(glm::mat4{ 1.f }, meshScale);
        local.m_position = meshScaleTransform * glm::vec4{ local.m_position, 1.f };

        return local.toMatrix();
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
        const util::Transform& offset,
        glm::vec3 meshScale)
        : m_name{ name },
        m_jointName{ jointName },
        m_offset{ offset },
        m_meshScale{ meshScale }
    {
        updateTransforms();
    }

    glm::mat4 RigSocket::calculateGlobalTransform(
        const glm::mat4& jointGlobalTransform) const
    {
        return m_meshScaleTransform *
            jointGlobalTransform *
            m_offsetTransform *
            m_invMeshScaleTransform;

        //return jointWorldTransform * m_transform;
    }

    void RigSocket::updateTransforms() {
        //m_transform = calculateLocalTransform(m_offset, m_meshScale);
        m_offsetTransform = m_offset.toMatrix();
        m_meshScaleTransform = calcMeshScaleTransform(m_meshScale);
        m_invMeshScaleTransform = glm::inverse(m_meshScaleTransform);
    }
}
