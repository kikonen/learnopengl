#include "RigSocket.h"

#include <glm/glm.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

namespace {
    glm::mat4 calcTransform(
        glm::vec3 offset,
        glm::quat rotation)
    {
        return glm::translate(glm::mat4{ 1.f }, offset) * glm::toMat4(rotation);
    }
}

namespace animation
{
    RigSocket::RigSocket(
        const std::string& name,
        const std::string& jointName,
        glm::vec3 offset,
        glm::quat rotation)
        : m_name{ name },
        m_jointName{ jointName },
        m_transform{ calcTransform(offset, rotation) }
    {}
}
