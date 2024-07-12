#include "BoneChannel.h"

#include <assimp/scene.h>

#include "util/assimp_util.h"

namespace {
    uint16_t findIndex(
        const std::vector<float>& times,
        float animationTimeTicks,
        uint16_t firstFrame,
        uint16_t lastFrame) noexcept
    {
        uint16_t min = firstFrame;
        uint16_t max = lastFrame;

        const auto startTick = times[firstFrame];

        // NOTE KI binary zearch
        while (min + 1 < max) {
            const auto curr = min + (max - min) / 2;

            if (animationTimeTicks < times[curr] - startTick) {
                max = curr;
            }
            else {
                min = curr;
            }
        }
        return min;
    }

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
        : m_jointName{ channel->mNodeName.C_Str() },
        m_jointIndex{ -1 }
    {}

    void BoneChannel::reservePositionKeys(uint16_t size)
    {
        m_positionKeyValues.reserve(size);
        m_positionKeyTimes.reserve(size);
    }

    void BoneChannel::reserveRotationKeys(uint16_t size)
    {
        m_rotationKeyValues.reserve(size);
        m_rotationKeyTimes.reserve(size);
    }

    void BoneChannel::reserveScaleKeys(uint16_t size)
    {
        m_scaleKeyValues.reserve(size);
        m_scaleKeyTimes.reserve(size);
    }

    void BoneChannel::addPositionKey(const aiVectorKey& key)
    {
        m_positionKeyValues.push_back(assimp_util::toVec3(key.mValue));
        m_positionKeyTimes.push_back(static_cast<float>(key.mTime));
    }

    void BoneChannel::addeRotationKey(const aiQuatKey& key)
    {
        m_rotationKeyValues.push_back(assimp_util::toQuat(key.mValue));
        m_rotationKeyTimes.push_back(static_cast<float>(key.mTime));
    }

    void BoneChannel::addeScaleKey(const aiVectorKey& key)
    {
        m_scaleKeyValues.push_back(assimp_util::toVec3(key.mValue));
        m_scaleKeyTimes.push_back(static_cast<float>(key.mTime));
    }

    // @return interpolated transform matrix
    glm::mat4 BoneChannel::interpolate(
        float animationTimeTicks,
        uint16_t firstFrame,
        uint16_t lastFrame,
        bool single) const noexcept
    {
        if (single) {
            firstFrame = 0;
        }

        //static const glm::mat4 ID_MAT{ 1.f };
        glm::mat4 s_translateMatrix{ 1.f };
        glm::mat4 s_scaleMatrix{ 1.f };

        {
            const auto& translate = interpolatePosition(animationTimeTicks, firstFrame, single ? m_positionKeyTimes.size() - 1 : lastFrame);
            s_translateMatrix[3].x = translate.x;
            s_translateMatrix[3].y = translate.y;
            s_translateMatrix[3].z = translate.z;
        }
        {
            const auto& scale = interpolateScale(animationTimeTicks, firstFrame, single ? m_scaleKeyTimes.size() - 1 : lastFrame);
            s_scaleMatrix[0].x = scale.x;
            s_scaleMatrix[1].y = scale.y;
            s_scaleMatrix[2].z = scale.z;
        }

        const auto& rotateMatrix = glm::toMat4(interpolateRotation(animationTimeTicks, firstFrame, single ? m_rotationKeyTimes.size() - 1 : lastFrame));

        return s_translateMatrix * rotateMatrix * s_scaleMatrix;
    }

    glm::vec3 BoneChannel::interpolatePosition(
        float animationTimeTicks,
        uint16_t firstFrame,
        uint16_t lastFrame) const noexcept
    {
        if (m_positionKeyValues.size() == 1) {
            return m_positionKeyValues[0];
        }

        uint16_t currIndex = findPosition(animationTimeTicks, firstFrame, lastFrame);
        uint16_t nextIndex = currIndex  + 1;

        assert(nextIndex < m_positionKeyValues.size());

        return interpolateVector(
            animationTimeTicks,
            m_positionKeyValues[currIndex],
            m_positionKeyValues[nextIndex],
            m_positionKeyTimes[firstFrame],
            m_positionKeyTimes[currIndex],
            m_positionKeyTimes[nextIndex]);
    }

    glm::quat BoneChannel::interpolateRotation(
        float animationTimeTicks,
        uint16_t firstFrame,
        uint16_t lastFrame) const noexcept
    {
        if (m_rotationKeyValues.size() == 1) {
            return m_rotationKeyValues[0];
        }

        uint16_t currIndex = findRotation(animationTimeTicks, firstFrame, lastFrame);
        uint16_t nextIndex = currIndex + 1;

        assert(nextIndex < m_rotationKeyValues.size());

        return interpolateQuaternion(
            animationTimeTicks,
            m_rotationKeyValues[currIndex],
            m_rotationKeyValues[nextIndex],
            m_rotationKeyTimes[firstFrame],
            m_rotationKeyTimes[currIndex],
            m_rotationKeyTimes[nextIndex]);
    }

    glm::vec3 BoneChannel::interpolateScale(
        float animationTimeTicks,
        uint16_t firstFrame,
        uint16_t lastFrame) const noexcept
    {
        if (m_scaleKeyValues.size() == 1) {
            return m_scaleKeyValues[0];
        }

        uint16_t currIndex = findScale(animationTimeTicks, firstFrame, lastFrame);
        uint16_t nextIndex = currIndex + 1;

        assert(nextIndex < m_scaleKeyValues.size());

        return interpolateVector(
            animationTimeTicks,
            m_scaleKeyValues[currIndex],
            m_scaleKeyValues[nextIndex],
            m_scaleKeyTimes[firstFrame],
            m_scaleKeyTimes[currIndex],
            m_scaleKeyTimes[nextIndex]);
    }

    glm::vec3 BoneChannel::interpolateVector(
        float animationTimeTicks,
        const glm::vec3& aValue,
        const glm::vec3& bValue,
        float firstFrameTime,
        float aTime,
        float bTime) const noexcept
    {
        const float t1 = (float)aTime - firstFrameTime;
        const float t2 = (float)bTime - firstFrameTime;
        const float deltaTime = t2 - t1;
        const float factor = (animationTimeTicks - t1) / deltaTime;

        // NOTE KI noticed strange negative -90 as time for some models
        // i.e. fbx/deinodonte/Deinodonte.FBX
        if (factor < 0.0f) return aValue;
        if (factor > 1.0f) return bValue;

        assert(factor >= 0.0f && factor <= 1.0f);

        const auto& start = aValue;
        const auto& end = bValue;

        auto delta = end - start;
        return start + factor * delta;
    }

    glm::quat BoneChannel::interpolateQuaternion(
        float animationTimeTicks,
        const glm::quat& aValue,
        const glm::quat& bValue,
        float firstFrameTime,
        float aTime,
        float bTime) const noexcept
    {
        const float t1 = (float)aTime - firstFrameTime;
        const float t2 = (float)bTime - firstFrameTime;
        const float deltaTime = t2 - t1;
        const float factor = (animationTimeTicks - t1) / deltaTime;

        // NOTE KI noticed strange negative -90 as time for some models
        // i.e. fbx/deinodonte/Deinodonte.FBX
        if (factor < 0.0f) return aValue;
        if (factor > 1.0f) return bValue;

        assert(factor >= 0.0f && factor <= 1.0f);

        return glm::slerp(aValue, bValue, factor);
    }

    uint16_t BoneChannel::findPosition(
        float animationTimeTicks,
        uint16_t firstFrame,
        uint16_t lastFrame) const noexcept
    {
        return findIndex(m_positionKeyTimes, animationTimeTicks, firstFrame, lastFrame);
    }

    uint16_t BoneChannel::findRotation(
        float animationTimeTicks,
        uint16_t firstFrame,
        uint16_t lastFrame) const noexcept
    {
        return findIndex(m_rotationKeyTimes, animationTimeTicks, firstFrame, lastFrame);
    }

    uint16_t BoneChannel::findScale(
        float animationTimeTicks,
        uint16_t firstFrame,
        uint16_t lastFrame) const noexcept
    {
        return findIndex(m_scaleKeyTimes, animationTimeTicks, firstFrame, lastFrame);
    }

    //uint16_t BoneChannel::findIndex2(
    //    const std::vector<float>& times,
    //    float animationTimeTicks) const noexcept
    //{
    //    for (uint16_t i = 0; i < times.size() - 1; i++) {
    //        if (animationTimeTicks < times[i + 1]) {
    //            return i;
    //        }
    //    }
    //    return 0;
    //}
}
