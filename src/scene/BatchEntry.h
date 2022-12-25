#pragma once

#include <glm/glm.hpp>

#pragma pack(push, 1)
struct BatchEntry {
    glm::mat4 modelMatrix;
    glm::mat3 normalMatrix;
    glm::vec4 objectID;
    float materialIndex;
};
#pragma pack(pop)
