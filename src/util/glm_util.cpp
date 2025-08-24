#include "glm_util.h"

#include <numbers>
#include <algorithm>

#include <glm/gtx/quaternion.hpp>

namespace {
    inline bool nearZero(float val, float epsilon = 0.00001f)
    {
        return fabs(val) <= epsilon;
    }
}

namespace util
{
    float degreesBetween(glm::vec3 a, glm::vec3 b)
    {
        return glm::degrees(radiansBetween(a, b));
    }

    float radiansBetween(glm::vec3 a, glm::vec3 b)
    {
        auto cosine = glm::dot(glm::normalize(a), glm::normalize(b));
        if (cosine == -1.f) return std::numbers::pi_v<float>;
        return acos(cosine);
        //return acos(cosine / (glm::length(a) * glm::length(b)));
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
        // TODO KI glm::angleAxis(radians, axis)
        return glm::angleAxis(radians, axis);

        //const auto v = glm::normalize(axis);
        //const auto halfAngle = radians / 2.f;
        //const auto sinHalf = sin(halfAngle);
        //return glm::normalize(glm::quat{
        //    cos(halfAngle),
        //    v.x * sinHalf,
        //    v.y * sinHalf,
        //    v.z * sinHalf
        //    });
    }

    // https://stackoverflow.com/questions/15873996/converting-a-direction-vector-to-a-quaternion-rotation
    // https://stackoverflow.com/questions/1171849/finding-quaternion-representing-the-rotation-from-one-vector-to-another/1171995#1171995
    // https://forums.unrealengine.com/t/rotation-from-normal/11543/3
    glm::quat normalToQuat(
        const glm::vec3& normal,
        const glm::vec3& up)
    {
        const auto& n = glm::normalize(normal);
        const auto& u = glm::normalize(up);

        const float thetaCos = glm::dot(u, n);

        // Identity rotation if exactly same dir (avoid NaN in acosf)
        if (thetaCos >= 0.99999f) return glm::quat{ 1.f, 0.f, 0.f, 0.f };

        // 180 rotation exactly opposite dir (avoid NaN in acosf)
        if (thetaCos <= -0.99999f) {
            return util::axisRadiansToQuat(normal, 0.f);
        }

        // https://stackoverflow.com/questions/1171849/finding-quaternion-representing-the-rotation-from-one-vector-to-another/1171995#1171995
        const auto a = glm::cross(u, n);
        auto w = 1.f + sqrt(thetaCos);

        return glm::normalize(glm::quat{ w, a.x, a.y, a.z });
    }

    // This will transform the vector and renormalize the w component
    glm::vec3 transformWithPerspDiv(
        const glm::vec3& vec,
        const glm::mat4& transform,
        float w /*= 1.0f*/)
    {
        glm::vec4 result = transform * glm::vec4(vec, w);

        float transformedW = result.w;
        if (!nearZero(std::abs(transformedW)))
        {
            transformedW = 1.0f / transformedW;
            result *= transformedW;
        }
        return result;
    }

    glm::mat4 getViewportMatrix(const glm::vec2& size)
    {
        const float w = size.x;
        const float h = size.y;
        const float w2 = w / 2.0f;
        const float h2 = h / 2.0f;

        return glm::mat4{
            glm::vec4{ w2, 0.0f, 0.0f, 0.0f },
            glm::vec4{ 0.0f, h2, 0.0f, 0.0f },
            glm::vec4{ 0.0f, 0.0f, 1.0f, 0.0f },
            glm::vec4{ w2 + 0, h2 + 0, 0.0f, 1.0f } };
    }

    void minmax(
        const glm::vec3& pos,
        glm::vec3& min,
        glm::vec3& max)
    {
        min.x = std::min(min.x, pos.x);
        min.y = std::min(min.y, pos.y);
        min.z = std::min(min.z, pos.z);

        max.x = std::max(max.x, pos.x);
        max.y = std::max(max.y, pos.y);
        max.z = std::max(max.z, pos.z);
    }

    // https://stackoverflow.com/questions/17918033/glm-decompose-mat4-into-translation-and-rotation
    void decomposeMtx(
        const glm::mat4& m,
        glm::vec3& pos,
        glm::quat& rot)
    {
        pos = m[3];
        rot = glm::quat_cast(m);
    }
}
