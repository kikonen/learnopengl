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
