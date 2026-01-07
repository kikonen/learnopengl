#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace util
{
    struct Transform;
}

struct aiNodeAnim;
struct aiVectorKey;
struct aiQuatKey;

namespace mesh_set
{
    class AnimationImporter;
}

namespace animation
{
    // Default LUT size (~1024 gives good quality vs memory trade-off)
    constexpr size_t DEFAULT_LUT_SIZE = 1024;

    // Vector transform/scale key frame
    struct VectorKey {
        VectorKey(const aiVectorKey& key);

        const glm::vec3 m_value;
        const float m_time;
    };

    // Quat roration key frame
    struct QuaternionKey {
        QuaternionKey(const aiQuatKey& key);

        const glm::quat m_value;
        const float m_time;
    };

    // Animation sequence for RigNode
    struct RigNodeChannel {
        friend class mesh_set::AnimationImporter;
        friend struct Animation;

        RigNodeChannel(const aiNodeAnim* channel);

        uint16_t getMaxFrame() const noexcept
        {
            return static_cast<uint16_t>(m_keyTimes.size() - 1);
        }

        void reservePositionKeys(uint16_t size);
        void reserveRotationKeys(uint16_t size);
        void reserveScaleKeys(uint16_t size);

        void addPositionKey(const aiVectorKey& key);
        void addeRotationKey(const aiQuatKey& key);
        void addeScaleKey(const aiVectorKey& key);

        // Unify position/rotation/scale to common timeline by resampling
        void unifyKeyTimes();

        // Generate lookup table for O(1) sampling
        // @param duration animation duration in ticks
        // @param lutSize number of entries in LUT (default 1024)
        void generateLUT(float duration, size_t lutSize = DEFAULT_LUT_SIZE);

        // O(1) sample from pre-computed LUT
        // @param normalizedTime time in range [0, 1] within clip
        void sampleLUT(
            float normalizedTime,
            util::Transform& transform) const noexcept;

        // @return true if LUT has been generated
        bool hasLUT() const noexcept { return !m_lutPositions.empty(); }

        // @param firstFrame..lastFrame range used for clip
        void interpolate(
            float animationTimeTicks,
            uint16_t firstFrame,
            uint16_t lastFrame,
            bool single,
            util::Transform& local) const noexcept;

    private:
        uint16_t findKeyIndex(
            float animationTimeTicks,
            uint16_t firstFrame,
            uint16_t lastFrame) const noexcept;

        // Helpers for resampling during unification
        static glm::vec3 sampleVector(
            const std::vector<glm::vec3>& values,
            const std::vector<float>& times,
            float t) noexcept;

        static glm::quat sampleQuaternion(
            const std::vector<glm::quat>& values,
            const std::vector<float>& times,
            float t) noexcept;

        const std::string m_nodeName;

        uint16_t m_index{ 0 };
        int16_t m_nodeIndex;

        // Unified key times (populated by unifyKeyTimes())
        std::vector<float> m_keyTimes;

        // Key values resampled to unified timeline
        std::vector<glm::vec3> m_positionKeyValues;
        std::vector<glm::quat> m_rotationKeyValues;
        std::vector<glm::vec3> m_scaleKeyValues;

        // Original key times (used during loading, cleared after unification)
        std::vector<float> m_positionKeyTimes;
        std::vector<float> m_rotationKeyTimes;
        std::vector<float> m_scaleKeyTimes;

        // Cached frame index for sequential playback optimization
        mutable uint16_t m_cachedKeyIndex{ 0 };

        // Pre-computed lookup table for O(1) sampling
        std::vector<glm::vec3> m_lutPositions;
        std::vector<glm::quat> m_lutRotations;
        std::vector<glm::vec3> m_lutScales;
        float m_lutInvScaleFactor{ 0.f };  // Maps [0,1] -> LUT index
    };
}
