#pragma once

#include <string>

#include <glad/glad.h>
#include "glm/glm.hpp"

// configure assets locations
class Assets
{
public: 
    std::string modelsDir = "3d_model";
    std::string shadersDir = "shader";

    glm::vec3 groundOffset = { 0.f, 15.f, -40.f };

    int shadowMapUnitId = GL_TEXTURE30;
    unsigned int shadowMapUnitIndex = 30;

    int skyboxUnitId = GL_TEXTURE31;
    unsigned int skyboxUnitIndex = 31;
};

