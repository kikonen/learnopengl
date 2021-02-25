#include "Assets.h"

#include <glad/glad.h>

Assets::Assets()
{
    glsl_version[0] = 4;
    glsl_version[1] = 5;
    glsl_version[2] = 0;

    glsl_version_str = "#version " + std::to_string(glsl_version[0]) + std::to_string(glsl_version[1]) + std::to_string(glsl_version[2]);

    modelsDir = "3d_model";
    shadersDir = "shader";
    spritesDir = "sprites";
    texturesDir = "textures";

    //groundOffset = { 0.f, 15.f, -40.f };
    groundOffset = { 200.f, 0.f, 200.f };

    terrainTileSize = 400;

    batchSize = 1000;

    nearPlane = 0.1f;
    farPlane = 1000.0f;

    refractionMapUnitIndex = 28;
    refractionMapUnitId = GL_TEXTURE0 + refractionMapUnitIndex;

    reflectionMapUnitIndex = 29;
    reflectionMapUnitId = GL_TEXTURE0 + reflectionMapUnitIndex;

    shadowMapUnitIndex = 30;
    shadowMapUnitId = GL_TEXTURE0 + shadowMapUnitIndex;

    skyboxUnitIndex = 31;
    skyboxUnitId = GL_TEXTURE0 + skyboxUnitIndex;
}
