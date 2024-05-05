#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace animation {
    // Matrix palette for single animation instance
    struct MatrixPalette {
        // index == index of bone in BonaContainer
        std::vector<glm::mat4> m_boneTransforms;

        bool m_dirty : 1 { false };
    };
}
