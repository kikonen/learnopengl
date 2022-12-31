#include "Assets.h"

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

    useIMGUI = false;
    useWrireframe = false;
    rasterizerDiscard = false;

    showNormals = false;
    showRearView = false;
    showShadowMapView = false;
    showReflectionView = false;
    showRefractionView = false;

    showSelectionWireframe = false;
    showVolume = false;
    showCubeMap = false;
    showHighlight = false;

    useLight = true;

    // NOTE KI no skipping of frames
    renderFrequency = 0.f;
    debugClearColor = false;
    clearColor = false;

    frustumEnabled = true;
    frustumDebug = false;

    cameraFov = 45.f;

    // NOTE KI mirror does not tolerate much skip
    mirrorRenderFrequency = 0.1f;
    mirrorFov = 40.f;

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
    cubeMapNearPlane = 0.5;
    cubeMapFarPlane = 200;
    cubeMapRenderFrequency = 0.2f;

    viewportEffect = ViewportEffect::none;

    volumeUUID = KI_UUID("9d409e0d-2716-48dd-a205-3a54bdfa5097");
    cubeMapUUID = KI_UUID("67a0f0f6-9e1a-4af3-97e9-a67eb11439d0");

    /////////////////////////
    // TEMPORARIES
}
