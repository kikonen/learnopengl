#include "glm_util.h"

#include <glm/gtx/quaternion.hpp>

namespace util
{
    glm::quat degreesToQuat(const glm::vec3& rot)
    {
        return glm::normalize(glm::quat(glm::radians(rot)));
    }

    glm::quat radiansToQuat(const glm::vec3& rot)
    {
        return glm::normalize(glm::quat(rot));
    }

    glm::vec3 quatToDegrees(const glm::quat& rot)
    {
        return glm::degrees(glm::eulerAngles(rot));
    }

    glm::vec3 quatToRadians(const glm::quat& rot)
    {
        return glm::eulerAngles(rot);
    }

    glm::vec3 radiansToDegrees(const glm::vec3& rot)
    {
        return glm::degrees(rot);
    }

    glm::vec3 degreesToRadians(const glm::vec3& rot)
    {
        return glm::radians(rot);
    }

    glm::quat axisDegreesToQuat(const glm::vec3& axis, float degrees)
    {
        return axisRadiansToQuat(axis, glm::radians(degrees));
    }

    // https://danceswithcode.net/engineeringnotes/quaternions/quaternions.html
    // https://gamedev.stackexchange.com/questions/149006/direction-vector-to-quaternion
    glm::quat axisRadiansToQuat(const glm::vec3& axis, float radians)
    {
        const auto v = glm::normalize(axis);
        const auto halfAngle = radians / 2.f;
        const auto sinHalf = sin(halfAngle);
        return glm::normalize(glm::quat{
            cos(halfAngle),
            v.x * sinHalf,
            v.y * sinHalf,
            v.z * sinHalf
            });
    }
}
