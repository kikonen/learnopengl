#pragma once

#include <glm/glm.hpp>

#pragma pack(push, 1)
struct BoneTransformSSBO {
    glm::mat4 u_transform;
};
#pragma pack(pop)
