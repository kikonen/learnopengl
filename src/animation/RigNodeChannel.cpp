#include "RigNodeChannel.h"

#include <algorithm>

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

    void RigNodeChannel::interpolate(
        float animationTimeTicks,
        uint16_t firstFrame,
        uint16_t lastFrame,
        bool single,
        util::Transform& transform) const noexcept
    {
        if (single) {
            firstFrame = 0;
            // NOTE KI lastFrame is exclusive, so use size (not size - 1)
            lastFrame = static_cast<uint16_t>(m_keyTimes.size());
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
        transform.m_position = glm::mix(m_positionKeyValues[currIndex], m_positionKeyValues[nextIndex], factor);

        // Interpolate rotation (slerp)
        transform.m_rotation = glm::slerp(
            m_rotationKeyValues[currIndex],
            m_rotationKeyValues[nextIndex],
            factor);

        // Interpolate scale (lerp)
        transform.m_scale = glm::mix(m_scaleKeyValues[currIndex], m_scaleKeyValues[nextIndex], factor);
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

    glm::vec3 RigNodeChannel::sampleOrigPosition(float tickTime) const noexcept
    {
        const auto& values = m_origPositionValues;
        const auto& times = m_origPositionTimes;

        if (values.empty()) return glm::vec3(0.f);
        if (values.size() == 1) return values[0];

        // Find surrounding keyframes
        size_t i = 0;
        while (i < times.size() - 1 && times[i + 1] <= tickTime) {
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

        const float factor = (tickTime - t0) / deltaTime;
        return values[i] + factor * (values[i + 1] - values[i]);
    }

    glm::quat RigNodeChannel::sampleOrigRotation(float tickTime) const noexcept
    {
        const auto& values = m_origRotationValues;
        const auto& times = m_origRotationTimes;

        if (values.empty()) return glm::quat(1.f, 0.f, 0.f, 0.f);
        if (values.size() == 1) return values[0];

        // Find surrounding keyframes
        size_t i = 0;
        while (i < times.size() - 1 && times[i + 1] <= tickTime) {
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

        const float factor = (tickTime - t0) / deltaTime;
        return glm::slerp(values[i], values[i + 1], factor);
    }

    glm::vec3 RigNodeChannel::sampleOrigScale(float tickTime) const noexcept
    {
        const auto& values = m_origScaleValues;
        const auto& times = m_origScaleTimes;

        if (values.empty()) return glm::vec3(1.f);
        if (values.size() == 1) return values[0];

        // Find surrounding keyframes
        size_t i = 0;
        while (i < times.size() - 1 && times[i + 1] <= tickTime) {
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

        const float factor = (tickTime - t0) / deltaTime;
        return values[i] + factor * (values[i + 1] - values[i]);
    }

    glm::vec3 RigNodeChannel::sampleOrigPositionClamped(float tickTime, float clipLastTick) const noexcept
    {
        const auto& values = m_origPositionValues;
        const auto& times = m_origPositionTimes;

        if (values.empty()) return glm::vec3(0.f);
        if (values.size() == 1) return values[0];

        // Find surrounding keyframes
        size_t i = 0;
        while (i < times.size() - 1 && times[i + 1] <= tickTime) {
            ++i;
        }

        if (i >= values.size() - 1) {
            return values.back();
        }

        const float t0 = times[i];
        const float t1 = times[i + 1];

        // If next keyframe is at or beyond clip boundary, don't interpolate with it
        // clipLastTick is exclusive (first tick NOT in clip), so use >=
        if (t1 >= clipLastTick) {
            // Also check if current keyframe is within clip
            if (t0 < clipLastTick) {
                return values[i];
            }
            // Current keyframe is also at/beyond boundary - use previous if available
            return i > 0 ? values[i - 1] : values[0];
        }

        const float deltaTime = t1 - t0;
        if (deltaTime <= 0.0f) {
            return values[i];
        }

        const float factor = (tickTime - t0) / deltaTime;
        return values[i] + factor * (values[i + 1] - values[i]);
    }

    glm::quat RigNodeChannel::sampleOrigRotationClamped(float tickTime, float clipLastTick) const noexcept
    {
        const auto& values = m_origRotationValues;
        const auto& times = m_origRotationTimes;

        if (values.empty()) return glm::quat(1.f, 0.f, 0.f, 0.f);
        if (values.size() == 1) return values[0];

        // Find surrounding keyframes
        size_t i = 0;
        while (i < times.size() - 1 && times[i + 1] <= tickTime) {
            ++i;
        }

        if (i >= values.size() - 1) {
            return values.back();
        }

        const float t0 = times[i];
        const float t1 = times[i + 1];

        // If next keyframe is at or beyond clip boundary, don't interpolate with it
        // clipLastTick is exclusive (first tick NOT in clip), so use >=
        if (t1 >= clipLastTick) {
            // Also check if current keyframe is within clip
            if (t0 < clipLastTick) {
                return values[i];
            }
            // Current keyframe is also at/beyond boundary - use previous if available
            return i > 0 ? values[i - 1] : values[0];
        }

        const float deltaTime = t1 - t0;
        if (deltaTime <= 0.0f) {
            return values[i];
        }

        const float factor = (tickTime - t0) / deltaTime;
        return glm::slerp(values[i], values[i + 1], factor);
    }

    glm::vec3 RigNodeChannel::sampleOrigScaleClamped(float tickTime, float clipLastTick) const noexcept
    {
        const auto& values = m_origScaleValues;
        const auto& times = m_origScaleTimes;

        if (values.empty()) return glm::vec3(1.f);
        if (values.size() == 1) return values[0];

        // Find surrounding keyframes
        size_t i = 0;
        while (i < times.size() - 1 && times[i + 1] <= tickTime) {
            ++i;
        }

        if (i >= values.size() - 1) {
            return values.back();
        }

        const float t0 = times[i];
        const float t1 = times[i + 1];

        // If next keyframe is at or beyond clip boundary, don't interpolate with it
        // clipLastTick is exclusive (first tick NOT in clip), so use >=
        if (t1 >= clipLastTick) {
            // Also check if current keyframe is within clip
            if (t0 < clipLastTick) {
                return values[i];
            }
            // Current keyframe is also at/beyond boundary - use previous if available
            return i > 0 ? values[i - 1] : values[0];
        }

        const float deltaTime = t1 - t0;
        if (deltaTime <= 0.0f) {
            return values[i];
        }

        const float factor = (tickTime - t0) / deltaTime;
        return values[i] + factor * (values[i + 1] - values[i]);
    }
}
