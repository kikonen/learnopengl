#pragma once

#include <glm/glm.hpp>

namespace util
{
    float ratiansBetween(glm::vec3 a, glm::vec3 b);

    glm::quat degreesToQuat(const glm::vec3& rot);
    glm::quat radiansToQuat(const glm::vec3& rot);

    glm::vec3 quatToDegrees(const glm::quat& rot);
    glm::vec3 quatToRadians(const glm::quat& rot);

    glm::vec3 radiansToDegrees(const glm::vec3& rot);
    glm::vec3 degreesToRadians(const glm::vec3& rot);

    glm::quat axisDegreesToQuat(const glm::vec3& axis, float degrees);
    glm::quat axisRadiansToQuat(const glm::vec3& axis, float radians);

    glm::quat normalToRotation(const glm::vec3& normal);
}
