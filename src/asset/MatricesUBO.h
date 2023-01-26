#pragma once

#include <glm/glm.hpp>


// NOTE KI align 16 for UBO struct
struct MatricesUBO {
    glm::mat4 projected;
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 viewSkybox;
    glm::mat4 lightProjected;
    glm::mat4 shadow;
};
