#include "Assets.h"

#include <glad/glad.h>

#include "ki/uuid.h"

Assets::Assets()
{
    glsl_version[0] = 4;
    glsl_version[1] = 5;
    glsl_version[2] = 0;

    glsl_version_str = "#version " + std::to_string(glsl_version[0]) + std::to_string(glsl_version[1]) + std::to_string(glsl_version[2]);

    glfwSwapInterval = 3;
    glDebug = false;

    resolutionScale = { 0.5, 0.5 };

    asyncLoaderEnabled = true;
    asyncLoaderDelay = 1000;

    logFile = "log/development.log";

    modelsDir = "3d_model";
    shadersDir = "shader";
    spritesDir = "sprites";
    texturesDir = "textures";

    placeholderTextureAlways = false;
    placeholderTexture = "textures/tiles.png";

    //groundOffset = { 0.f, 15.f, -40.f };
    //groundOffset = { 200.f, 0.f, 200.f };

    useIMGUI = false;

    showNormals = false;
    showMirrorView = false;
    showShadowMapView = false;
    showReflectionView = false;
    showRefractionView = false;

    // NOTE KI no skipping of frames
    renderFrequency = 0.f;
    debugClearColor = false;
    clearColor = false;

    frustumEnabled = true;
    frustumDebug = false;

    // NOTE KI mirror does not tolerate much skip
    mirrorRenderFrequency = 0.1f;

    waterTileSize = 100;
    // NOTE KI water tolerates less skip than shadow/cube
    // => i.e. it's "sharper" thus lack is more visible to user
    waterRenderFrequency = 0.1f;

    terrainVertexCount = 64;
    terrainTileSize = 400;

    batchSize = 10;

    nearPlane = 0.1f;
    farPlane = 1000.0f;

    fogColor = glm::vec4(0.1, 0.1, 0.2, 1.0);
    fogStart = 50.0;
    fogEnd = 700.0;

    shadowNearPlane = 0.1f;
    shadowFarPlane = 1000.0f;
    shadowMapSize = 1024;
    shadowRenderFrequency = 0.2f;

    mirrorReflectionSize = 1000;
    mirrorRefractionSize = 1000;

    waterReflectionSize = 1000;
    waterRefractionSize = 1000;

    cubeMapSize = 1000;
    cubeMapRenderFrequency = 0.2f;

    cubeMapNearPlane = 0.5;
    cubeMapFarPlane = 200;

    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glActiveTexture.xhtml
    noiseUnitIndex = 64;
    mirrorReflectionMapUnitIndex = 65;
    waterRefractionMapUnitIndex = 66;
    waterReflectionMapUnitIndex = 67;
    cubeMapUnitIndex = 68;
    shadowMapUnitIndex = 69;
    skyboxUnitIndex = 70;

    volumeUUID = KI_UUID("9d409e0d-2716-48dd-a205-3a54bdfa5097");

    /////////////////////////
    // TEMPORARIES
}
