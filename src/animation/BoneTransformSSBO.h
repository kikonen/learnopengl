#pragma once

#include <glm/glm.hpp>

namespace animation {
#pragma pack(push, 1)
    struct BoneTransformSSBO {
        glm::mat4 u_transform;
    };
#pragma pack(pop)
}
