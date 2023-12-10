#include "glm_util.h"

#include <glm/gtx/quaternion.hpp>

namespace util
{
    glm::quat degreesToQuat(const glm::vec3& rot)
    {
        return glm::quat(glm::radians(rot));
    }

    glm::quat radiansToQuat(const glm::vec3& rot)
    {
        return glm::quat(rot);
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
}
