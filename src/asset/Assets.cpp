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

    resolutionScale = { 0.5, 0.5 };

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
    useWrireframe = false;
    rasterizerDiscard = false;

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

    useLight = true;

    debugClearColor = false;
    clearColor = false;

    frustumEnabled = true;
    frustumCPU = false;
    frustumCPU = true;
    frustumDebug = false;

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

    terrainVertexCount = 64;
    terrainTileSize = 512;

    batchSize = 16;
    batchBuffers = 3;

    nearPlane = 0.1f;
    farPlane = 1000.0f;

    fogColor = glm::vec4(0.1, 0.1, 0.2, 1.0);
    fogStart = 50.0;
    fogEnd = 700.0;

    shadowNearPlane = 0.1f;
    shadowFarPlane = 1000.0f;
    shadowFrustumSize = 100.0f;
    shadowMapSize = 1024;

    shadowRenderFrameStart = 0;
    shadowRenderFrameStep = 2;

    mirrorReflectionSize = 1024;
    mirrorRefractionSize = 1024;

    waterReflectionSize = 1024;
    waterRefractionSize = 1024;

    cubeMapSize = 1024;
    cubeMapNearPlane = 0.5;
    cubeMapFarPlane = 200;

    cubeMapRenderFrameStart = 1;
    cubeMapRenderFrameStep = 2;

    viewportEffect = ViewportEffect::none;

    rootUUID =    KI_UUID("11111111-1111-1111-1111-111111111111");
    volumeUUID =  KI_UUID("11111111-1111-1111-1111-222222222222");
    cubeMapUUID = KI_UUID("11111111-1111-1111-1111-333333333333");

    /////////////////////////
    // TEMPORARIES
}
