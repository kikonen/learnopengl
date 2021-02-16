#pragma once

#include <string>

#include <glad/glad.h>
#include "glm/glm.hpp"

// configure assets locations
class Assets final
{
public: 
    std::string modelsDir = "3d_model";
    std::string shadersDir = "shader";
    std::string spritesDir = "sprites";
    std::string texturesDir = "textures";

    //glm::vec3 groundOffset = { 0.f, 15.f, -40.f };
    glm::vec3 groundOffset = { 200.f, 0.f, 200.f };

    int terrainTileSize = 400;

    int batchSize = 1000;

    float nearPlane = 0.1f;
    float farPlane = 1000.0f;

    int refactionMapUnitId = GL_TEXTURE28;
    unsigned int refactionMapUnitIndex = 28;

    int reflectionMapUnitId = GL_TEXTURE29;
    unsigned int reflectionMapUnitIndex = 29;

    int shadowMapUnitId = GL_TEXTURE30;
    unsigned int shadowMapUnitIndex = 30;

    int skyboxUnitId = GL_TEXTURE31;
    unsigned int skyboxUnitIndex = 31;
};

