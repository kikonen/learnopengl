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

    void RigNodeChannel::generateLUT(float duration, size_t lutSize)
    {
        if (m_keyTimes.empty() || duration <= 0.f || lutSize < 2) {
            return;
        }

        m_lutPositions.resize(lutSize);
        m_lutRotations.resize(lutSize);
        m_lutScales.resize(lutSize);

        // Compute scale factor: maps normalized time [0,1] to LUT index [0, lutSize-1]
        m_lutInvScaleFactor = static_cast<float>(lutSize - 1);

        const float startTime = m_keyTimes.front();
        const float endTime = m_keyTimes.back();
        const float timeRange = endTime - startTime;

        if (timeRange <= 0.f) {
            // Static pose - fill with first keyframe
            for (size_t i = 0; i < lutSize; ++i) {
                m_lutPositions[i] = m_positionKeyValues[0];
                m_lutRotations[i] = m_rotationKeyValues[0];
                m_lutScales[i] = m_scaleKeyValues[0];
            }
            return;
        }

        // Sample at each LUT entry
        size_t keyIndex = 0;
        for (size_t i = 0; i < lutSize; ++i) {
            // Normalized time for this LUT entry
            const float normalizedTime = static_cast<float>(i) / static_cast<float>(lutSize - 1);
            const float sampleTime = startTime + normalizedTime * timeRange;

            // Advance keyIndex to find surrounding keyframes
            while (keyIndex < m_keyTimes.size() - 2 && m_keyTimes[keyIndex + 1] < sampleTime) {
                ++keyIndex;
            }

            const size_t currIdx = keyIndex;
            const size_t nextIdx = keyIndex + 1;

            const float t0 = m_keyTimes[currIdx];
            const float t1 = m_keyTimes[nextIdx];
            const float deltaTime = t1 - t0;

            float factor = 0.f;
            if (deltaTime > 0.f) {
                factor = (sampleTime - t0) / deltaTime;
                factor = std::clamp(factor, 0.f, 1.f);
            }

            // Interpolate and store in LUT
            m_lutPositions[i] = m_positionKeyValues[currIdx] +
                factor * (m_positionKeyValues[nextIdx] - m_positionKeyValues[currIdx]);

            m_lutRotations[i] = glm::slerp(
                m_rotationKeyValues[currIdx],
                m_rotationKeyValues[nextIdx],
                factor);

            m_lutScales[i] = m_scaleKeyValues[currIdx] +
                factor * (m_scaleKeyValues[nextIdx] - m_scaleKeyValues[currIdx]);
        }

        // Clear original keyframe data (LUT replaces it)
        m_keyTimes.clear();
        m_keyTimes.shrink_to_fit();
        m_positionKeyValues.clear();
        m_positionKeyValues.shrink_to_fit();
        m_rotationKeyValues.clear();
        m_rotationKeyValues.shrink_to_fit();
        m_scaleKeyValues.clear();
        m_scaleKeyValues.shrink_to_fit();
    }

    void RigNodeChannel::sampleLUT(
        float normalizedTime,
        util::Transform& transform) const noexcept
    {
        if (m_lutPositions.empty()) {
            // Fallback if no LUT
            return;
        }

        // Clamp to valid range
        normalizedTime = std::clamp(normalizedTime, 0.f, 1.f);

        // O(1) index calculation
        const float exactIndex = normalizedTime * m_lutInvScaleFactor;
        const size_t idx0 = static_cast<size_t>(exactIndex);
        const size_t idx1 = std::min(idx0 + 1, m_lutPositions.size() - 1);
        const float factor = exactIndex - static_cast<float>(idx0);

        // Interpolate between adjacent LUT entries for smooth animation
        transform.m_position = m_lutPositions[idx0] +
            factor * (m_lutPositions[idx1] - m_lutPositions[idx0]);

        transform.m_rotation = glm::slerp(m_lutRotations[idx0], m_lutRotations[idx1], factor);

        transform.m_scale = m_lutScales[idx0] +
            factor * (m_lutScales[idx1] - m_lutScales[idx0]);
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
