#include "NodeInstance.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


void NodeInstance::setRotation(const glm::vec3& rotation) noexcept
{
    m_rotation = rotation;
    m_rotationMatrix = glm::toMat4(glm::quat(glm::radians(m_rotation)));
}

const glm::mat4& NodeInstance::compile(const glm::mat4& parentMatrix) const
{
    const static glm::mat4 IDENTITY_MATRIX{ 1.f };

    //return parentMatrix *
    //    glm::translate(IDENTITY_MATRIX, m_position) *
    //    glm::toMat4(glm::quat(glm::radians(m_rotation))) *
    //    glm::scale(IDENTITY_MATRIX, m_scale);

    return parentMatrix *
        glm::scale(
            glm::translate(IDENTITY_MATRIX, m_position) * m_rotationMatrix,
            m_scale);
}
