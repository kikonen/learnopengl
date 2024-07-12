#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct aiNodeAnim;
struct aiVectorKey;
struct aiQuatKey;

namespace animation {
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

    // Animation sequence for Joint
    struct BoneChannel {
        friend class AnimationLoader;
        friend struct Animation;

        BoneChannel(const aiNodeAnim* channel);

        uint16_t getMaxFrame() const noexcept
        {
            return static_cast<uint16_t>(m_positionKeyTimes.size() - 1);
        }

        void reservePositionKeys(uint16_t size);
        void reserveRotationKeys(uint16_t size);
        void reserveScaleKeys(uint16_t size);

        void addPositionKey(const aiVectorKey& key);
        void addeRotationKey(const aiQuatKey& key);
        void addeScaleKey(const aiVectorKey& key);

        // @param firstFrame..lastFrame range used for clip
        glm::mat4 interpolate(
            float animationTimeTicks,
            uint16_t firstFrame,
            uint16_t lastFrame) const noexcept;

    private:
        glm::vec3 interpolatePosition(
            float animationTimeTicks,
            uint16_t firstFrame,
            uint16_t lastFrame) const noexcept;

        glm::quat interpolateRotation(
            float animationTimeTicks,
            uint16_t firstFrame,
            uint16_t lastFrame) const noexcept;

        glm::vec3 interpolateScale(
            float animationTimeTicks,
            uint16_t firstFrame,
            uint16_t lastFrame) const noexcept;

        glm::vec3 interpolateVector(
            float animationTimeTicks,
            const glm::vec3& aValue,
            const glm::vec3& bValue,
            float firstFrameTime,
            float aTime,
            float bTime) const noexcept;

        glm::quat interpolateQuaternion(
            float animationTimeTicks,
            const glm::quat& aValue,
            const glm::quat& bValue,
            float firstFrameTime,
            float aTime,
            float bTime) const noexcept;

        uint16_t findPosition(
            float animationTimeTicks,
            uint16_t firstFrame,
            uint16_t lastFrame) const noexcept;

        uint16_t findRotation(
            float animationTimeTicks,
            uint16_t firstFrame,
            uint16_t lastFrame) const noexcept;

        uint16_t findScale(
            float animationTimeTicks,
            uint16_t firstFrame,
            uint16_t lastFrame) const noexcept;

        //uint16_t findIndex(
        //    const std::vector<float>& times,
        //    float animationTimeTicks) const noexcept;

        const std::string m_jointName;

        uint16_t m_index{ 0 };
        int16_t m_jointIndex;

        std::vector<glm::vec3> m_positionKeyValues;
        std::vector<float> m_positionKeyTimes;

        std::vector<glm::quat> m_rotationKeyValues;
        std::vector<float> m_rotationKeyTimes;

        std::vector<glm::vec3> m_scaleKeyValues;
        std::vector<float> m_scaleKeyTimes;
    };
}
