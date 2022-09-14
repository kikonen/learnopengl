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

    placeHolderTextureAlways = false;
    placeHolderTexture = "textures/tiles.png";

    //groundOffset = { 0.f, 15.f, -40.f };
    groundOffset = { 200.f, 0.f, 200.f };

    // NOTE KI no skipping of frames
    drawSkip = 0;
    debugClearColor = true;
    clearColor = false;

    waterTileSize = 100;

    terrainVertexCount = 64;
    terrainTileSize = 400;

    batchSize = 1000;

    nearPlane = 0.1f;
    farPlane = 1000.0f;

    fogColor = glm::vec4(0.1, 0.1, 0.2, 1.0);
    fogStart = 50.0;
    fogEnd = 700.0;

    shadowNearPlane = 0.1f;
    shadowFarPlane = 1000.0f;
    shadowMapSize = 1024;

    mirrorReflectionSize = 1000;
    mirrorRefractionSize = 1000;

    waterReflectionSize = 1000;
    waterRefractionSize = 1000;

    cubeMapSize = 1000;
    cubeMapDrawSkip = 1;

    noiseUnitIndex = 24;
    mirrorRefractionMapUnitIndex = 25;
    mirrorReflectionMapUnitIndex = 26;
    waterRefractionMapUnitIndex = 27;
    waterReflectionMapUnitIndex = 28;
    cubeMapUnitIndex = 29;
    shadowMapUnitIndex = 30;
    skyboxUnitIndex = 31;
}
