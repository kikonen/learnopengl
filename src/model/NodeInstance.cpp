#include "NodeInstance.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


const glm::mat4& NodeInstance::compile(const glm::mat4& parentMatrix) const
{
    const static glm::mat4 IDENTITY_MATRIX{ 1.f };

    return glm::translate(IDENTITY_MATRIX, m_position) *
        glm::toMat4(glm::quat(glm::radians(m_rotation))) *
        glm::scale(IDENTITY_MATRIX, m_scale);
}
