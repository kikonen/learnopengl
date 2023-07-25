#pragma once

#include <glm/glm.hpp>

// NOTE KI align 16 for UBO struct
#pragma pack(push, 1)
struct BufferInfoUBO {
    // NOTE KI std140
    // "Both the size and alignment are twice"
    // "the size of the underlying scalar type."
    glm::vec2 u_bufferResolution;

    int pad1_1;
    int pad1_2;
};
#pragma pack(pop)
