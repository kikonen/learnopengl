#pragma once

#include <glm/glm.hpp>

namespace animation {
    // Joint weights for single vertex
    struct VertexJoint {
        glm::uvec4 m_jointIds{ 0 };
        glm::vec4 m_weights{ 0.f };

        // Insert joint data into first free slot
        // - free == weight not yet set
        void addJoint(uint16_t jointId, float weight) {
            if (weight == 0.f)
                return;
            assert(weight > 0 && weight <= 1.f);

            for (int i = 0; i < 4; i++) {
                // NOTE KI jointId *CAN* be zero
                //if (m_jointIds[i] == jointId)
                //    return;

                if (m_weights[i] == 0.f) {
                    m_jointIds[i] = jointId;
                    m_weights[i] = weight;
                    break;
                }
                else {
                    if (m_jointIds[i] == jointId) {
                        m_weights[i] += weight;
                        break;
                        //assert(m_jointIds[i] != jointId);
                    }
                }
            }
            float sum = m_weights.x + m_weights.y + m_weights.z + m_weights.w;
            assert(sum <= 1.0001f);
            //assert(0);
        }
    };
}
