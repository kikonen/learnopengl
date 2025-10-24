#pragma once

#include <string>
#include <vector>
#include <map>

#include <glm/glm.hpp>

#include "ki/size.h"

struct aiBone;

namespace animation {
    // NOTE KI Assimp aiBone == Joint
    struct Joint {
        Joint(const aiBone* bone);

        int16_t m_index{ -1 };

        int16_t m_nodeIndex{ -1 };
        std::string m_nodeName;

        /** FROM ASSIMP
         * Matrix that transforms from mesh space to bone space in bind pose.
         *
         * This matrix describes the position of the mesh
         * in the local space of this bone when the skeleton was bound.
         * Thus it can be used directly to determine a desired vertex position,
         * given the world-space transform of the bone when animated,
         * and the position of the vertex in mesh space.
         *
         * It is sometimes called an inverse-bind matrix,
         * or inverse bind pose matrix.
         */
        glm::mat4 m_offsetMatrix;
    };
}
