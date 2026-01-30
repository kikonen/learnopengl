#include "ClipChannelLUT.h"

#include <algorithm>

#include <fmt/format.h>

#include "util/Log.h"
#include "RigNodeChannel.h"
#include "util/Transform.h"

namespace animation
{
    void ClipChannelLUT::generate(
        const RigNodeChannel& channel,
        const std::string& clipName,
        float firstTick,
        float lastTick,
        size_t lutSize)
    {
        if (lutSize < 2 || firstTick >= lastTick) {
            return;
        }

        // Check that original tracks exist
        if (channel.getOrigPositionValues().empty() &&
            channel.getOrigRotationValues().empty() &&
            channel.getOrigScaleValues().empty()) {
            return;
        }

        m_positions.resize(lutSize);
        m_rotations.resize(lutSize);
        m_scales.resize(lutSize);

        // Compute scale factor: maps normalized time [0,1] to LUT index [0, lutSize-1]
        m_invScaleFactor = static_cast<float>(lutSize - 1);

        const float tickRange = lastTick - firstTick;

        if (tickRange <= 0.f) {
            // Static pose - sample at firstTick
            glm::vec3 pos = channel.sampleOrigPosition(firstTick);
            glm::quat rot = channel.sampleOrigRotation(firstTick);
            glm::vec3 scale = channel.sampleOrigScale(firstTick);

            for (size_t i = 0; i < lutSize; ++i) {
                m_positions[i] = pos;
                m_rotations[i] = rot;
                m_scales[i] = scale;
            }
            return;
        }

        // Debug: show keyframe times near clip boundaries
        const auto& origTimes = channel.getOrigPositionTimes();
        if (!origTimes.empty()) {
            // Find keyframes near lastTick
            float lastKeyBefore = -1.f, firstKeyAfter = -1.f;
            for (size_t k = 0; k < origTimes.size(); ++k) {
                if (origTimes[k] <= lastTick) lastKeyBefore = origTimes[k];
                if (origTimes[k] > lastTick && firstKeyAfter < 0.f) firstKeyAfter = origTimes[k];
            }

            KI_DEBUG(fmt::format(
                "ASSIMP::LUT_GEN: clip={}, ticks=[{:.1f},{:.1f}] lastKeyBefore={:.1f} firstKeyAfter={:.1f} totalKeys={}",
                clipName, firstTick, lastTick, lastKeyBefore, firstKeyAfter, origTimes.size()));
        }

        // Sample at each LUT entry using original tracks
        // This avoids cross-clip interpolation artifacts from unified timeline
        for (size_t i = 0; i < lutSize; ++i) {
            // Normalized time for this LUT entry
            const float normalizedTime = static_cast<float>(i) / static_cast<float>(lutSize - 1);
            const float sampleTick = firstTick + normalizedTime * tickRange;

            // Sample with clip boundary clamping to prevent cross-clip interpolation
            m_positions[i] = channel.sampleOrigPositionClamped(sampleTick, lastTick);
            m_rotations[i] = channel.sampleOrigRotationClamped(sampleTick, lastTick);
            m_scales[i] = channel.sampleOrigScaleClamped(sampleTick, lastTick);
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
