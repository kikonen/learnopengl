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

    glm::mat4 RigSocket::calculateGlobalTransform(
        const glm::mat4& jointGlobalTransform) const
    {
        // Scale-neutral socket transform: meshScale * joint * invMeshScale
        // This cancels parent mesh scale so attached nodes aren't affected
        // Then offset is applied in world-scaled space (intuitive units)
        // i.e., offset of 0.1 means 0.1 actual units, regardless of mesh scale
        return m_meshScaleTransform *
            jointGlobalTransform *
            m_invMeshScaleTransform *
            m_offsetTransform;
    }

    void RigSocket::updateTransforms() {
        // NOTE KI offset.scale is expected to be 1 (only position+rotation used)
        // Scale comes from meshScale which converts mesh coords to world coords
        m_offsetTransform = m_offset.toMatrix();
        m_meshScaleTransform = calcMeshScaleTransform(m_meshScale);
        m_invMeshScaleTransform = glm::inverse(m_meshScaleTransform);
    }
}
