#pragma once

#include <glm/glm.hpp>

namespace animation {
    // Bone weights for single vertex
    struct VertexBone {
        glm::uvec4 m_boneIds{ 0 };
        glm::vec4 m_weights{ 0.f };

        // Insert bone data into first free slot
        // - free == weight not yet set
        void addBone(uint16_t boneId, float weight) {
            if (weight == 0.f)
                return;

            for (int i = 0; i < 4; i++) {
                // NOTE KI boneId *CAN* be zero
                //if (m_boneIds[i] == boneId)
                //    return;

                if (m_weights[i] == 0.f) {
                    m_boneIds[i] = boneId;
                    m_weights[i] = weight;
                    return;
                }
            }
            assert(0);
        }
    };
}
