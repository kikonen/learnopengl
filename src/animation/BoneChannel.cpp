#include "BoneChannel.h"

#include <assimp/scene.h>

#include "util/assimp_util.h"

namespace {
}

namespace animation {
    VectorKey::VectorKey(const aiVectorKey& key)
        : m_time{ static_cast<float>(key.mTime) },
        m_value{ assimp_util::toVec3(key.mValue) }
    {}

    QuaternionKey::QuaternionKey(const aiQuatKey& key)
        : m_time{ static_cast<float>(key.mTime) },
        m_value{ assimp_util::toQuat(key.mValue) }
    {}

    BoneChannel::BoneChannel(const aiNodeAnim* channel)
        : m_nodeName{ channel->mNodeName.C_Str() },
        m_nodeIndex{ -1 }
    {}

    // @return interpolated transform matrix
    glm::mat4 BoneChannel::interpolate(float animationTimeTicks) const noexcept
    {
        const glm::mat4 ID_MAT{ 1.f };

        auto scaleMat = glm::scale(ID_MAT, interpolateScale(animationTimeTicks));
        auto rotateMat = glm::toMat4(interpolateRotation(animationTimeTicks));
        auto translateMat = glm::translate(ID_MAT, interpolatePosition(animationTimeTicks));

        return translateMat * rotateMat * scaleMat;
    }

    glm::vec3 BoneChannel::interpolatePosition(float animationTimeTicks) const noexcept
    {
        if (m_positionKeys.size() == 1) {
            return m_positionKeys[0].m_value;
        }

        uint16_t currIndex = findPosition(animationTimeTicks);
        uint16_t nextIndex = currIndex  + 1;

        assert(nextIndex < m_positionKeys.size());

        return interpolateVector(
            animationTimeTicks,
            m_positionKeys[currIndex],
            m_positionKeys[nextIndex]);
    }

    glm::quat BoneChannel::interpolateRotation(float animationTimeTicks) const noexcept
    {
        if (m_rotationKeys.size() == 1) {
            return m_rotationKeys[0].m_value;
        }

        uint16_t currIndex = findRotation(animationTimeTicks);
        uint16_t nextIndex = currIndex + 1;

        assert(nextIndex < m_rotationKeys.size());

        return interpolateQuaternion(
            animationTimeTicks,
            m_rotationKeys[currIndex],
            m_rotationKeys[nextIndex]);
    }

    glm::vec3 BoneChannel::interpolateScale(float animationTimeTicks) const noexcept
    {
        if (m_scaleKeys.size() == 1) {
            return m_scaleKeys[0].m_value;
        }

        uint16_t currIndex = findScale(animationTimeTicks);
        uint16_t nextIndex = currIndex + 1;

        assert(nextIndex < m_scaleKeys.size());

        return interpolateVector(
            animationTimeTicks,
            m_scaleKeys[currIndex],
            m_scaleKeys[nextIndex]);
    }

    glm::vec3 BoneChannel::interpolateVector(
        float animationTimeTicks,
        const VectorKey& a,
        const VectorKey& b) const noexcept
    {
        const float t1 = (float)a.m_time;
        const float t2 = (float)b.m_time;
        const float deltaTime = t2 - t1;
        const float factor = (animationTimeTicks - t1) / deltaTime;

        assert(factor >= 0.0f && factor <= 1.0f);

        const auto& start = a.m_value;
        const auto& end = b.m_value;

        auto delta = end - start;
        return start + factor * delta;
    }

    glm::quat BoneChannel::interpolateQuaternion(
        float animationTimeTicks,
        const QuaternionKey& a,
        const QuaternionKey& b) const noexcept
    {
        const float t1 = (float)a.m_time;
        const float t2 = (float)b.m_time;
        const float deltaTime = t2 - t1;
        const float factor = (animationTimeTicks - t1) / deltaTime;

        assert(factor >= 0.0f && factor <= 1.0f);

        return glm::slerp(a.m_value, b.m_value, factor);
    }

    uint16_t BoneChannel::findPosition(float animationTimeTicks) const noexcept
    {
        for (uint16_t i = 0; i < m_positionKeys.size() - 1; i++) {
            float t = (float)m_positionKeys[i + 1].m_time;
            if (animationTimeTicks < t) {
                return i;
            }
        }
        return 0;
    }

    uint16_t BoneChannel::findRotation(float animationTimeTicks) const noexcept
    {
        for (uint16_t i = 0; i < m_rotationKeys.size() - 1; i++) {
            float t = (float)m_rotationKeys[i + 1].m_time;
            if (animationTimeTicks < t) {
                return i;
            }
        }
        return 0;
    }

    uint16_t BoneChannel::findScale(float animationTimeTicks) const noexcept
    {
        for (uint16_t i = 0; i < m_scaleKeys.size() - 1; i++) {
            float t = (float)m_scaleKeys[i + 1].m_time;
            if (animationTimeTicks < t) {
                return i;
            }
        }
        return 0;
    }
}
