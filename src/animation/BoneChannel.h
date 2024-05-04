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

        const float m_time;
        const glm::vec3 m_value;
    };

    // Quat roration key frame
    struct QuaternionKey {
        QuaternionKey(const aiQuatKey& key);

        const float m_time;
        const glm::quat m_value;
    };

    // Animation sequence for Node
    struct BoneChannel {
        BoneChannel(const aiNodeAnim* channel);

        glm::mat4 interpolate(float animationTimeTicks) const noexcept;

        glm::vec3 interpolatePosition(float animationTimeTicks) const noexcept;
        glm::quat interpolateRotation(float animationTimeTicks) const noexcept;
        glm::vec3 interpolateScale(float animationTimeTicks) const noexcept;

        glm::vec3 interpolateVector(
            float animationTimeTicks,
            const VectorKey& a,
            const VectorKey& b) const noexcept;

        glm::quat interpolateQuaternion(
            float animationTimeTicks,
            const QuaternionKey& a,
            const QuaternionKey& b) const noexcept;

        uint16_t findPosition(float animationTimeTicks) const noexcept;
        uint16_t findRotation(float animationTimeTicks) const noexcept;
        uint16_t findScale(float animationTimeTicks) const noexcept;

        const std::string m_nodeName;
        //const aiNodeAnim* m_channel;

        uint16_t m_index{ 0 };
        int16_t m_nodeIndex;

        std::vector<VectorKey> m_positionKeys;
        std::vector<QuaternionKey> m_rotationKeys;
        std::vector<VectorKey> m_scaleKeys;
    };
}
