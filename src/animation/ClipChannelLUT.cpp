#include "ClipChannelLUT.h"

#include <algorithm>

#include "RigNodeChannel.h"
#include "util/Transform.h"

namespace animation
{
    void ClipChannelLUT::generate(
        const RigNodeChannel& channel,
        uint16_t firstFrame,
        uint16_t lastFrame,
        size_t lutSize)
    {
        const auto& keyTimes = channel.getKeyTimes();
        const auto& positions = channel.getPositionValues();
        const auto& rotations = channel.getRotationValues();
        const auto& scales = channel.getScaleValues();

        if (keyTimes.empty() || lutSize < 2 || firstFrame >= lastFrame || lastFrame < 1) {
            return;
        }

        // Clamp frame indices to valid range
        firstFrame = std::min(firstFrame, static_cast<uint16_t>(keyTimes.size() - 1));
        lastFrame = std::min(lastFrame, static_cast<uint16_t>(keyTimes.size() - 1));

        if (firstFrame >= lastFrame) {
            return;
        }

        m_positions.resize(lutSize);
        m_rotations.resize(lutSize);
        m_scales.resize(lutSize);

        // Compute scale factor: maps normalized time [0,1] to LUT index [0, lutSize-1]
        m_invScaleFactor = static_cast<float>(lutSize - 1);

        const float startTime = keyTimes[firstFrame];
        const float endTime = keyTimes[lastFrame];
        const float timeRange = endTime - startTime;

        if (timeRange <= 0.f) {
            // Static pose - fill with first keyframe
            for (size_t i = 0; i < lutSize; ++i) {
                m_positions[i] = positions[firstFrame];
                m_rotations[i] = rotations[firstFrame];
                m_scales[i] = scales[firstFrame];
            }
            return;
        }

        // Sample at each LUT entry
        size_t keyIndex = firstFrame;
        for (size_t i = 0; i < lutSize; ++i) {
            // Normalized time for this LUT entry
            const float normalizedTime = static_cast<float>(i) / static_cast<float>(lutSize - 1);
            const float sampleTime = startTime + normalizedTime * timeRange;

            // Advance keyIndex to find surrounding keyframes
            while (keyIndex + 1 < lastFrame && keyTimes[keyIndex + 1] < sampleTime) {
                ++keyIndex;
            }

            const size_t currIdx = keyIndex;
            const size_t nextIdx = std::min(keyIndex + 1, static_cast<size_t>(lastFrame));

            const float t0 = keyTimes[currIdx];
            const float t1 = keyTimes[nextIdx];
            const float deltaTime = t1 - t0;

            float factor = 0.f;
            if (deltaTime > 0.f) {
                factor = (sampleTime - t0) / deltaTime;
                factor = std::clamp(factor, 0.f, 1.f);
            }

            // Interpolate and store in LUT
            m_positions[i] = positions[currIdx] +
                factor * (positions[nextIdx] - positions[currIdx]);

            m_rotations[i] = glm::slerp(
                rotations[currIdx],
                rotations[nextIdx],
                factor);

            m_scales[i] = scales[currIdx] +
                factor * (scales[nextIdx] - scales[currIdx]);
        }
    }

    void ClipChannelLUT::sample(
        float normalizedTime,
        util::Transform& transform) const noexcept
    {
        if (m_positions.empty()) {
            return;
        }

        // O(1) direct lookup - no interpolation needed with 1024 entries
        const size_t idx = static_cast<size_t>(
            std::clamp(normalizedTime, 0.f, 1.f) * m_invScaleFactor);

        transform.m_position = m_positions[idx];
        transform.m_rotation = m_rotations[idx];
        transform.m_scale = m_scales[idx];
    }
}
