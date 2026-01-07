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

        // Accessors for ClipChannelLUT generation
        int16_t getNodeIndex() const noexcept { return m_nodeIndex; }
        const std::vector<float>& getKeyTimes() const noexcept { return m_keyTimes; }
        const std::vector<glm::vec3>& getPositionValues() const noexcept { return m_positionKeyValues; }
        const std::vector<glm::quat>& getRotationValues() const noexcept { return m_rotationKeyValues; }
        const std::vector<glm::vec3>& getScaleValues() const noexcept { return m_scaleKeyValues; }

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
    };
}
