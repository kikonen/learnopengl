#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct aiNodeAnim;
struct aiVectorKey;
struct aiQuatKey;

namespace animation {
    struct VectorKey {
        VectorKey(const aiVectorKey& key);

        float m_time;
        glm::vec3 m_value;
    };

    struct QuaternionKey {
        QuaternionKey(const aiQuatKey& key);

        float m_time;
        glm::quat m_value;
    };

    struct BoneChannel {
        BoneChannel(const aiNodeAnim* channel);

        const std::string m_nodeName;
        const aiNodeAnim* m_channel;

        std::vector<VectorKey> m_positionKeys;
        std::vector<QuaternionKey> m_rotationKeys;
        std::vector<VectorKey> m_scalingKeys;
    };
}
