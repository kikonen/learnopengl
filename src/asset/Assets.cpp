#include "Assets.h"

#include <fmt/format.h>

#include "ki/GL.h"
#include "ki/uuid.h"

Assets::Assets()
{
    glsl_version[0] = 4;
    glsl_version[1] = 6;
    glsl_version[2] = 0;

    glsl_version_str = fmt::format(
        "#version {}{}{}",
        glsl_version[0], glsl_version[1], glsl_version[2]);

    glPreferredTextureFormatRGBA = GL_RGBA8;
    glPreferredTextureFormatRGB = GL_RGB8;

    glfwSwapInterval = 3;
    glDebug = false;
    glNoError = false;

    resolutionScale = 0.5f;
    gbufferScale = 0.5f;
    bufferScale = 0.5f;

    cameraMoveNormal = { 4.5, 4.5, 4.5 };
    cameraMoveRun = { 8, 8, 8 };
    cameraRotateNormal = { 12, 12, 12};
    cameraRotateRun = { 20, 20, 20 };
    cameraZoomNormal = { 20, 20, 20 };
    cameraZoomRun = { 25, 25, 25 };
    cameraMouseSensitivity = { 0.1, 0.1, 0.1 };

    asyncLoaderEnabled = true;
    asyncLoaderDelay = 1000;

    logFile = "log/development.log";
    sceneFile = "scene/scene_full.yml";

    modelsDir = "3d_model";
    shadersDir = "shader";
    spritesDir = "sprites";
    texturesDir = "textures";

    placeholderTextureAlways = false;
    placeholderTexture = "textures/tiles_1024.png";

    useIMGUI = false;

    useScript = true;
    useLight = true;

    forceWireframe = false;

    rasterizerDiscard = false;

    renderCubeMap = true;
    renderShadowMap = true;
    renderMirrorMap = true;
    renderWaterMap = true;

    showNormals = false;
    showRearView = false;
    showShadowMapView = false;
    showReflectionView = false;
    showRefractionView = false;

    showVolume = false;
    showSelectionVolume = false;

    showHighlight = false;
    showSelection = false;
    showCubeMapCenter = false;
    showTagged = false;

    debugClearColor = false;
    clearColor = false;

    frustumEnabled = true;
    frustumCPU = false;
    frustumCPU = true;
    frustumAny = frustumEnabled && (frustumCPU || frustumGPU);

    frustumDebug = false;
    frustumVisual = false;

    cameraFov = 45.f;

    // NOTE KI no skipping of frames
    renderFrameStart = 0;
    renderFrameStep = 0;

    nodeRenderFrameStart = 0;
    nodeRenderFrameStep = 0;

    // NOTE KI mirror does not tolerate much skip
    mirrorRenderFrameStart = 0;
    mirrorRenderFrameStep = 2;

    waterTileSize = 128;
    // NOTE KI water tolerates less skip than shadow/cube
    // => i.e. it's "sharper" thus lack is more visible to user
    waterRenderFrameStart = 0;
    waterRenderFrameStep = 2;

    terrainGridSize = 32;

    batchSize = 16;
    batchBuffers = 3;
    batchDebug = false;

    drawDebug = false;

    nearPlane = 0.1f;
    farPlane = 1000.0f;

    fogColor = glm::vec4(0.1, 0.1, 0.2, 1.0);
    fogStart = 50.0;
    fogEnd = 700.0;
    fogDensity = 1.0;

    shadowPolygonOffsetEnabled = true;
    shadowPolygonOffset = { 2.f, 2.f };

    shadowPlanes = { 0.1f, 20.f, 50.f, 100.f };
    shadowMapSizes = { 1024, 512, 512 };

    shadowRenderFrameStart = 0;
    shadowRenderFrameStep = 2;

    mirrorReflectionSize = 1024;
    mirrorRefractionSize = 1024;

    waterReflectionSize = 1024;
    waterRefractionSize = 1024;

    cubeMapSeamless = true;

    cubeMapSize = 1024;
    cubeMapNearPlane = 0.5;
    cubeMapFarPlane = 200;

    cubeMapRenderFrameStart = 1;
    cubeMapRenderFrameStep = 2;

    viewportEffectEnabled = false;
    viewportEffect = ViewportEffect::none;

    computeGroups = { 1, 1, 1 };

    rootUUID =    KI_UUID("11111111-1111-1111-1111-111111111111");
    volumeUUID =  KI_UUID("11111111-1111-1111-1111-222222222222");
    cubeMapUUID = KI_UUID("11111111-1111-1111-1111-333333333333");
    skyboxUUID =  KI_UUID("11111111-1111-1111-1111-444444444444");

    /////////////////////////
    // TEMPORARIES
}
