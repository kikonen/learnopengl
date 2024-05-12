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

    // Animation sequence for Node
    struct BoneChannel {
        friend class AnimationLoader;
        friend struct Animation;

        BoneChannel(const aiNodeAnim* channel);

        void reservePositionKeys(uint16_t size);
        void reserveRotationKeys(uint16_t size);
        void reserveScaleKeys(uint16_t size);

        void addPositionKey(const aiVectorKey& key);
        void addeRotationKey(const aiQuatKey& key);
        void addeScaleKey(const aiVectorKey& key);

        glm::mat4 interpolate(float animationTimeTicks) const noexcept;

    private:
        glm::vec3 interpolatePosition(float animationTimeTicks) const noexcept;
        glm::quat interpolateRotation(float animationTimeTicks) const noexcept;
        glm::vec3 interpolateScale(float animationTimeTicks) const noexcept;

        glm::vec3 interpolateVector(
            float animationTimeTicks,
            const glm::vec3& aValue,
            const glm::vec3& bValue,
            float aTime,
            float bTime) const noexcept;

        glm::quat interpolateQuaternion(
            float animationTimeTicks,
            const glm::quat& aValue,
            const glm::quat& bValue,
            float aTime,
            float bTime) const noexcept;

        uint16_t findPosition(float animationTimeTicks) const noexcept;
        uint16_t findRotation(float animationTimeTicks) const noexcept;
        uint16_t findScale(float animationTimeTicks) const noexcept;

        uint16_t findIndex(
            const std::vector<float>& times,
            float animationTimeTicks) const noexcept;

        const std::string m_nodeName;

        uint16_t m_index{ 0 };
        int16_t m_nodeIndex;

        std::vector<glm::vec3> m_positionKeyValues;
        std::vector<float> m_positionKeyTimes;

        std::vector<glm::quat> m_rotationKeyValues;
        std::vector<float> m_rotationKeyTimes;

        std::vector<glm::vec3> m_scaleKeyValues;
        std::vector<float> m_scaleKeyTimes;
    };
}
