#pragma once

#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace util
{
    struct Transform;
}

namespace animation
{
    struct RigNodeChannel;

    // Default LUT size (~1024 gives good quality vs memory trade-off)
    constexpr size_t DEFAULT_LUT_SIZE = 1024;

    // Pre-computed lookup table for a single (clip, channel) pair
    // Enables O(1) animation sampling at runtime
    struct ClipChannelLUT
    {
        ClipChannelLUT() = default;

        // Generate LUT by sampling channel's original tracks for the clip's tick range
        // Uses original (non-unified) tracks to avoid cross-clip interpolation artifacts
        // @param channel source channel with original keyframes
        // @param firstTick clip's first tick value (inclusive)
        // @param lastTick clip's last tick value (inclusive)
        void generate(
            const RigNodeChannel& channel,
            const std::string& clipName,
            float firstTick,
            float lastTick,
            size_t lutSize = DEFAULT_LUT_SIZE);

        // O(1) sample from pre-computed LUT
        // @param normalizedTime time in range [0, 1] within clip
        void sample(
            float normalizedTime,
            util::Transform& transform) const noexcept;

        bool empty() const noexcept { return m_positions.empty(); }

    private:
        std::vector<glm::vec3> m_positions;
        std::vector<glm::quat> m_rotations;
        std::vector<glm::vec3> m_scales;
        float m_invScaleFactor{ 0.f };  // Maps [0,1] -> LUT index
    };
}
