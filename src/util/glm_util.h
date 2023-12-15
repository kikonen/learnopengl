#pragma once

#include <glm/glm.hpp>

namespace util
{
    glm::quat degreesToQuat(const glm::vec3& rot);
    glm::quat radiansToQuat(const glm::vec3& rot);

    glm::vec3 quatToDegrees(const glm::quat& rot);
    glm::vec3 quatToRadians(const glm::quat& rot);

    glm::vec3 radiansToDegrees(const glm::vec3& rot);
    glm::vec3 degreesToRadians(const glm::vec3& rot);

    glm::quat degreesDirToQuat(const glm::vec3& dir, float degrees);
}
