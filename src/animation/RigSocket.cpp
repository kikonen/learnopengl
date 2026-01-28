#include "RigSocket.h"

#include <glm/glm.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

namespace {
    glm::mat4 calcMeshScaleTransform(glm::vec3 meshScale)
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

    glm::mat4 RigSocket::calculateScaleNeutralGlobalTransform(
        const glm::mat4& jointGlobalTransform) const
    {
        // Scale-neutral joint transform (converts joint from mesh coords to world coords)
        // meshScale * joint * invMeshScale cancels scale for attached nodes
        return m_meshScaleTransform *
            jointGlobalTransform *
            m_invMeshScaleTransform;
    }

    glm::mat4 RigSocket::calculateGlobalTransform(
        const glm::mat4& jointGlobalTransform) const
    {
        // Apply offset in joint-local space
        // - Offset directions follow joint axes (visible in debug visualization)
        // - Offset values are in world units (0.1 = 0.1 meters)
        // - Attached nodes follow bone animation
        return calculateScaleNeutralGlobalTransform(jointGlobalTransform) * m_offsetTransform;
    }

    void RigSocket::updateTransforms() {
        // NOTE KI offset.scale is expected to be 1 (only position+rotation used)
        // Offset is applied in joint-local space (directions follow joint axes)
        // Values are in world units due to scale-neutral joint transform
        m_offsetTransform = m_offset.toMatrix();
        m_meshScaleTransform = calcMeshScaleTransform(m_meshScale);
        m_invMeshScaleTransform = glm::inverse(m_meshScaleTransform);
    }
}
