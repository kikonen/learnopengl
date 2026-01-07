#include "RigNodeChannel.h"

#include <algorithm>
#include <set>

#include <assimp/scene.h>

#include "util/assimp_util.h"
#include "util/Transform.h"

namespace {
    uint16_t binarySearchIndex(
        const std::vector<float>& times,
        float animationTimeTicks,
        uint16_t firstFrame,
        uint16_t lastFrame) noexcept
    {
        uint16_t min = firstFrame;
        // NOTE KI -1 so that "next = curr + 1" won't blend into next clip
        uint16_t max = lastFrame - 1;

        const auto startTick = times[firstFrame];

        // NOTE KI binary search
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

    RigNodeChannel::RigNodeChannel(const aiNodeAnim* channel)
        : m_nodeName{ assimp_util::normalizeName(channel->mNodeName) },
        m_nodeIndex{ -1 }
    {}

    void RigNodeChannel::reservePositionKeys(uint16_t size)
    {
        m_positionKeyValues.reserve(size);
        m_positionKeyTimes.reserve(size);
    }

    void RigNodeChannel::reserveRotationKeys(uint16_t size)
    {
        m_rotationKeyValues.reserve(size);
        m_rotationKeyTimes.reserve(size);
    }

    void RigNodeChannel::reserveScaleKeys(uint16_t size)
    {
        m_scaleKeyValues.reserve(size);
        m_scaleKeyTimes.reserve(size);
    }

    void RigNodeChannel::addPositionKey(const aiVectorKey& key)
    {
        m_positionKeyValues.push_back(assimp_util::toVec3(key.mValue));
        m_positionKeyTimes.push_back(static_cast<float>(key.mTime));
    }

    void RigNodeChannel::addeRotationKey(const aiQuatKey& key)
    {
        m_rotationKeyValues.push_back(assimp_util::toQuat(key.mValue));
        m_rotationKeyTimes.push_back(static_cast<float>(key.mTime));
    }

    void RigNodeChannel::addeScaleKey(const aiVectorKey& key)
    {
        m_scaleKeyValues.push_back(assimp_util::toVec3(key.mValue));
        m_scaleKeyTimes.push_back(static_cast<float>(key.mTime));
    }

    glm::vec3 RigNodeChannel::sampleVector(
        const std::vector<glm::vec3>& values,
        const std::vector<float>& times,
        float t) noexcept
    {
        if (values.size() == 1) {
            return values[0];
        }

        // Find surrounding keyframes
        size_t i = 0;
        while (i < times.size() - 1 && times[i + 1] <= t) {
            ++i;
        }

        if (i >= values.size() - 1) {
            return values.back();
        }

        const float t0 = times[i];
        const float t1 = times[i + 1];
        const float deltaTime = t1 - t0;

        if (deltaTime <= 0.0f) {
            return values[i];
        }

        const float factor = (t - t0) / deltaTime;
        return values[i] + factor * (values[i + 1] - values[i]);
    }

    glm::quat RigNodeChannel::sampleQuaternion(
        const std::vector<glm::quat>& values,
        const std::vector<float>& times,
        float t) noexcept
    {
        if (values.size() == 1) {
            return values[0];
        }

        // Find surrounding keyframes
        size_t i = 0;
        while (i < times.size() - 1 && times[i + 1] <= t) {
            ++i;
        }

        if (i >= values.size() - 1) {
            return values.back();
        }

        const float t0 = times[i];
        const float t1 = times[i + 1];
        const float deltaTime = t1 - t0;

        if (deltaTime <= 0.0f) {
            return values[i];
        }

        const float factor = (t - t0) / deltaTime;
        return glm::slerp(values[i], values[i + 1], factor);
    }

    void RigNodeChannel::unifyKeyTimes()
    {
        // Collect all unique time points from position, rotation, scale
        std::set<float> uniqueTimes;

        for (float t : m_positionKeyTimes) uniqueTimes.insert(t);
        for (float t : m_rotationKeyTimes) uniqueTimes.insert(t);
        for (float t : m_scaleKeyTimes) uniqueTimes.insert(t);

        // Build unified timeline
        m_keyTimes.assign(uniqueTimes.begin(), uniqueTimes.end());

        // Resample each track to unified timeline
        std::vector<glm::vec3> newPositions;
        std::vector<glm::quat> newRotations;
        std::vector<glm::vec3> newScales;

        newPositions.reserve(m_keyTimes.size());
        newRotations.reserve(m_keyTimes.size());
        newScales.reserve(m_keyTimes.size());

        for (float t : m_keyTimes) {
            newPositions.push_back(sampleVector(m_positionKeyValues, m_positionKeyTimes, t));
            newRotations.push_back(sampleQuaternion(m_rotationKeyValues, m_rotationKeyTimes, t));
            newScales.push_back(sampleVector(m_scaleKeyValues, m_scaleKeyTimes, t));
        }

        // Replace values with resampled data
        m_positionKeyValues = std::move(newPositions);
        m_rotationKeyValues = std::move(newRotations);
        m_scaleKeyValues = std::move(newScales);

        // Clear original time vectors (no longer needed)
        m_positionKeyTimes.clear();
        m_positionKeyTimes.shrink_to_fit();
        m_rotationKeyTimes.clear();
        m_rotationKeyTimes.shrink_to_fit();
        m_scaleKeyTimes.clear();
        m_scaleKeyTimes.shrink_to_fit();
    }

    void RigNodeChannel::interpolate(
        float animationTimeTicks,
        uint16_t firstFrame,
        uint16_t lastFrame,
        bool single,
        util::Transform& transform) const noexcept
    {
        if (single) {
            firstFrame = 0;
            lastFrame = static_cast<uint16_t>(m_keyTimes.size() - 1);
        }

        if (m_keyTimes.size() == 1) {
            transform.m_position = m_positionKeyValues[0];
            transform.m_rotation = m_rotationKeyValues[0];
            transform.m_scale = m_scaleKeyValues[0];
            return;
        }

        const uint16_t currIndex = findKeyIndex(animationTimeTicks, firstFrame, lastFrame);
        const uint16_t nextIndex = currIndex + 1;

        const float startTick = m_keyTimes[firstFrame];
        const float t1 = m_keyTimes[currIndex] - startTick;
        const float t2 = m_keyTimes[nextIndex] - startTick;
        const float deltaTime = t2 - t1;
        const float factor = (animationTimeTicks - t1) / deltaTime;

        // NOTE KI handle edge cases (strange negative times in some models)
        if (factor < 0.0f) {
            transform.m_position = m_positionKeyValues[currIndex];
            transform.m_rotation = m_rotationKeyValues[currIndex];
            transform.m_scale = m_scaleKeyValues[currIndex];
            return;
        }
        if (factor > 1.0f) {
            transform.m_position = m_positionKeyValues[nextIndex];
            transform.m_rotation = m_rotationKeyValues[nextIndex];
            transform.m_scale = m_scaleKeyValues[nextIndex];
            return;
        }

        // Interpolate position (lerp)
        transform.m_position = m_positionKeyValues[currIndex] +
            factor * (m_positionKeyValues[nextIndex] - m_positionKeyValues[currIndex]);

        // Interpolate rotation (slerp)
        transform.m_rotation = glm::slerp(
            m_rotationKeyValues[currIndex],
            m_rotationKeyValues[nextIndex],
            factor);

        // Interpolate scale (lerp)
        transform.m_scale = m_scaleKeyValues[currIndex] +
            factor * (m_scaleKeyValues[nextIndex] - m_scaleKeyValues[currIndex]);
    }

    uint16_t RigNodeChannel::findKeyIndex(
        float animationTimeTicks,
        uint16_t firstFrame,
        uint16_t lastFrame) const noexcept
    {
        const float startTick = m_keyTimes[firstFrame];
        const uint16_t maxIndex = lastFrame - 1;

        // Check if cached index is still valid (common case for sequential playback)
        if (m_cachedKeyIndex >= firstFrame &&
            m_cachedKeyIndex < maxIndex &&
            animationTimeTicks >= m_keyTimes[m_cachedKeyIndex] - startTick &&
            animationTimeTicks < m_keyTimes[m_cachedKeyIndex + 1] - startTick)
        {
            return m_cachedKeyIndex;
        }

        m_cachedKeyIndex = binarySearchIndex(m_keyTimes, animationTimeTicks, firstFrame, lastFrame);
        return m_cachedKeyIndex;
    }
}
