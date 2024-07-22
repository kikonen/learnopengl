#include "glm_util.h"

#include <glm/gtx/quaternion.hpp>

namespace util
{
    float ratiansBetween(glm::vec3 a, glm::vec3 b)
    {
        return acos(glm::dot(a, b) / (glm::length(a) * glm::length(b)));
    }

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

    // https://forums.unrealengine.com/t/rotation-from-normal/11543/3
    glm::quat normalToRotation(const glm::vec3& normal)
    {
        glm::vec3 up{ 0, 1.f, 0 };

        const float thetaCos = glm::dot(up, normal);

        // Identity rotation if exactly same dir (avoid NaN in acosf)
        if (thetaCos == 1.f) return glm::quat{ 0, 0, 0, 1.f };

        const auto axis = glm::normalize(glm::cross(up, normal));
        const float theta = acosf(thetaCos);

        return util::axisRadiansToQuat(axis, theta);
    }
}
