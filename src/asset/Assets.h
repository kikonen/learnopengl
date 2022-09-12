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

    std::string logFile;

    std::string modelsDir;
    std::string shadersDir;
    std::string spritesDir;
    std::string texturesDir;

    bool placeHolderTextureAlways;
    std::string placeHolderTexture;

    glm::vec3 groundOffset;

    int waterTileSize;

    int terrainVertexCount;
    int terrainTileSize;

    int batchSize;

    float nearPlane;
    float farPlane;

    glm::vec4 fogColor;
    float fogStart;
    float fogEnd;

    // NOTE KI MUST match lookup() in light shadow shader
    float shadowNearPlane;
    float shadowFarPlane;
    int shadowMapSize;

    int mirrorReflectionSize;
    int mirrorRefractionSize;

    int waterReflectionSize;
    int waterRefractionSize;

    int cubeMapSize;

    unsigned int noiseUnitIndex;
    unsigned int mirrorRefractionMapUnitIndex;
    unsigned int mirrorReflectionMapUnitIndex;
    unsigned int waterRefractionMapUnitIndex;
    unsigned int waterReflectionMapUnitIndex;
    unsigned int cubeMapUnitIndex;
    unsigned int shadowMapUnitIndex;
    unsigned int skyboxUnitIndex;
};

