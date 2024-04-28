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

        glm::vec3 interpolatePosition(float animationTimeTicks);
        glm::quat interpolateRotation(float animationTimeTicks);
        glm::vec3 interpolateScale(float animationTimeTicks);

        glm::vec3 interpolateVector(
            float animationTimeTicks,
            const VectorKey& a,
            const VectorKey& b);

        glm::quat interpolateQuaternion(
            float animationTimeTicks,
            const QuaternionKey& a,
            const QuaternionKey& b);

        uint16_t findPosition(float animationTimeTicks);
        uint16_t findRotation(float animationTimeTicks);
        uint16_t findScale(float animationTimeTicks);

        const std::string m_nodeName;
        //const aiNodeAnim* m_channel;

        uint16_t m_id;
        int16_t m_nodeId;

        std::vector<VectorKey> m_positionKeys;
        std::vector<QuaternionKey> m_rotationKeys;
        std::vector<VectorKey> m_scaleKeys;
    };
}
