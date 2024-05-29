#pragma once

#include <glm/glm.hpp>

namespace animation {
    // Bone weights for single vertex
    struct VertexBone {
        glm::uvec4 m_boneIds;
        glm::vec4 m_weights;

        uint8_t index{ 0 };

        void addBone(uint16_t boneId, float weight) {
            assert(index < 4);
            // NOTE KI precaution avoid memory corruption in release mode
            if (index == 4)
                return;
            m_boneIds[index] = boneId;
            m_weights[index] = weight;
            index++;
        }
    };
}
