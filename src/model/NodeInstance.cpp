#include "NodeInstance.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


void NodeInstance::setRotation(const glm::vec3& rotation) noexcept
{
    m_rotation = rotation;
    m_rotationMatrix = glm::toMat4(glm::quat(glm::radians(m_rotation)));
}
