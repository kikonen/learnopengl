#pragma once

#include <glm/glm.hpp>

namespace mesh {
#pragma pack(push, 1)
    struct TransformSSBO {
        glm::mat4 u_transform;
    };
#pragma pack(pop)
}
