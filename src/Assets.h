#pragma once

#include <string>

#include <glad/glad.h>
//#include <GLFW/glfw3.h>

// configure assets locations
class Assets
{
public: 
    std::string modelsDir = "3d_model";
    std::string shadersDir = "shader";

    int depthMapUnitId = GL_TEXTURE30;
    unsigned int depthMapUnitIndex = 30;

    int skyboxUnitId = GL_TEXTURE31;
    unsigned int skyboxUnitIndex = 31;
};

