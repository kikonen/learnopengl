#pragma once

#include <string>

#include "glm/glm.hpp"

// configure assets locations
class Assets final
{
public:
    Assets();

public: 
    int glsl_version[3];
    std::string glsl_version_str;

    std::string modelsDir;
    std::string shadersDir;
    std::string spritesDir;
    std::string texturesDir;

    glm::vec3 groundOffset;

    int terrainTileSize;

    int batchSize;

    float nearPlane;
    float farPlane;

    int refractionMapUnitId;
    unsigned int refractionMapUnitIndex;

    int reflectionMapUnitId;
    unsigned int reflectionMapUnitIndex;

    int shadowMapUnitId;
    unsigned int shadowMapUnitIndex;

    int skyboxUnitId;
    unsigned int skyboxUnitIndex;
};

