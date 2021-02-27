#include "Assets.h"

#include <glad/glad.h>

Assets::Assets()
{
    glsl_version[0] = 4;
    glsl_version[1] = 5;
    glsl_version[2] = 0;

    glsl_version_str = "#version " + std::to_string(glsl_version[0]) + std::to_string(glsl_version[1]) + std::to_string(glsl_version[2]);

    logFile = "log/development.log";

    modelsDir = "3d_model";
    shadersDir = "shader";
    spritesDir = "sprites";
    texturesDir = "textures";

    //groundOffset = { 0.f, 15.f, -40.f };
    groundOffset = { 200.f, 0.f, 200.f };

    waterTileSize = 100;
    terrainTileSize = 400;

    batchSize = 1000;

    nearPlane = 0.1f;
    farPlane = 1000.0f;

    shadowNearPlane = 0.1f;
    shadowFarPlane = 1000.0f;
    shadowMapSize = 1000;

    waterReflectionSize = 1000;
    waterRefractionSize = 1000;

    reflectionCubeSize = 1000;
    refractionCubeSize = 1000;

    waterRefractionMapUnitIndex = 26;
    waterRefractionMapUnitId = GL_TEXTURE0 + waterRefractionMapUnitIndex;

    waterReflectionMapUnitIndex = 27;
    waterReflectionMapUnitId = GL_TEXTURE0 + waterReflectionMapUnitIndex;

    refractionMapUnitIndex = 28;
    refractionMapUnitId = GL_TEXTURE0 + refractionMapUnitIndex;

    reflectionMapUnitIndex = 29;
    reflectionMapUnitId = GL_TEXTURE0 + reflectionMapUnitIndex;

    shadowMapUnitIndex = 30;
    shadowMapUnitId = GL_TEXTURE0 + shadowMapUnitIndex;

    skyboxUnitIndex = 31;
    skyboxUnitId = GL_TEXTURE0 + skyboxUnitIndex;
}
