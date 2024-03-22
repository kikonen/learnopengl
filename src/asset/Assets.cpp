#include "Assets.h"

#include <filesystem>

#include <fmt/format.h>

#include "ki/sid.h"

#include "kigl/kigl.h"

namespace {
    static Assets s_assets;
}

const Assets& Assets::get() noexcept {
    return s_assets;
}

Assets& Assets::modify() noexcept {
    return s_assets;
}

void Assets::set(const Assets& assets) noexcept
{
    s_assets = assets;
}

Assets::Assets()
{
    glsl_version[0] = 4;
    glsl_version[1] = 6;
    glsl_version[2] = 0;

    glsl_version_str = fmt::format(
        "#version {}{}{}",
        glsl_version[0], glsl_version[1], glsl_version[2]);

    glVendorNvidia = false;
    glVendorIntel = false;

    glPreferredTextureFormatRGBA = GL_RGBA8;
    glPreferredTextureFormatRGB = GL_RGB8;

    glfwSwapInterval = 3;
    glDebug = false;
    glNoError = false;

    glUseMapped = false;
    glUseInvalidate = true;
    glUseFence = false;
    glUseSingleFence = false;
    glUseDebugFence = false;
    glUseFinish = false;

    compressedTexturesEnabled = false;
    prepassDepthEnabled = false;

    gBufferScale = 0.5f;
    waterReflectionBufferScale = 0.125f;
    waterRefractionBufferScale = 0.25f;
    mirrorReflectionBufferScale = 0.25f;

    windowSize = { 800, 600 };
    windowMaximized = false;
    windowFullScreen = false;

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

    sceneDir = "scene";
    sceneFile = "scene_full.yml";

    rootDir = std::filesystem::current_path().string();

    assetsDir = "resources/assets";
    modelsDir = "{{assets_dir}}";
    spritesDir = "{{assets_dir}}/sprites";
    texturesDir = "{{assets_dir}}/textures";
    fontsDir = "{{assets_dir}}/fonts";

    shadersDir = "shader";

    placeholderTextureAlways = false;
    placeholderTexture = "textures/tiles_1024.png";

    useImGui = false;
    imGuiDemo = false;
    imGuiFontSize = 12;
    imGuiFontPath = "fonts/Vera.ttf";

    useScript = true;
    useLight = true;

    forceWireframe = false;

    rasterizerDiscard = false;

    showNormals = false;
    showRearView = false;
    showShadowMapView = false;
    showReflectionView = false;
    showRefractionView = false;
    showObjectIDView = false;

    showVolume = false;
    showSelectionVolume = false;

    showHighlight = false;
    showSelection = false;
    showCubeMapCenter = false;
    showTagged = false;

    useDebugColor = false;

    frustumEnabled = true;
    frustumGPU = false;
    frustumCPU = true;
    frustumAny = frustumEnabled && (frustumCPU || frustumGPU);
    frustumParallelLimit = 999;

    frustumDebug = false;

    cameraFov = 45.f;

    // NOTE KI no skipping of frames
    renderFrameStart = 0;
    renderFrameStep = 0;

    nodeRenderFrameStart = 0;
    nodeRenderFrameStep = 0;

    mirrorMapEnabled = true;
    mirrorReflectionSize = 1024;
    mirrorFov = 30.f;

    mirrorRenderMirror = true;
    mirrorRenderWater = true;

    // NOTE KI mirror does not tolerate much skip
    mirrorRenderFrameStart = 0;
    mirrorRenderFrameStep = 2;

    mirrorMapNearPlane = 0.1f;
    mirrorMapFarPlane = 1000;

    waterMapEnabled = true;
    waterTileSize = 128;
    // NOTE KI water tolerates less skip than shadow/cube
    // => i.e. it's "sharper" thus lack is more visible to user
    waterRenderFrameStart = 0;
    waterRenderFrameStep = 2;

    waterMapNearPlane = 0.1f;
    waterMapFarPlane = 1000;

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

    particleEnabled = true;
    particleMaxCount = 100000;

    shadowMapEnabled = true;

    shadowPolygonOffsetEnabled = true;
    shadowPolygonOffset = { 2.f, 2.f };

    shadowPlanes = { 0.1f, 20.f, 50.f, 100.f };
    shadowMapSizes = { 1024, 512, 512 };

    shadowRenderFrameStart = 0;
    shadowRenderFrameStep = 2;

    shadowVisual = false;

    cubeMapEnabled = true;
    cubeMapSeamless = true;
    cubeMapSkipOthers = true;
    cubeMapSize = 1024;
    cubeMapNearPlane = 0.5;
    cubeMapFarPlane = 200;

    cubeMapRenderMirror = true;
    cubeMapRenderWater = true;

    cubeMapRenderFrameStart = 1;
    cubeMapRenderFrameStep = 2;

    skyboxEnabled = true;
    skyboxSize = 1024;

    environmentMapEnabled = true;
    environmentMapSize = 512;
    irradianceMapSize = 32;
    prefilterMapSize = 128;
    brdfLutSize = 512;

    viewportEffectEnabled = false;
    viewportEffect = ViewportEffect::none;

    hdrGamma = 2.2f;
    hdrExposure = 1.0f;

    effectBloomEnabled = true;
    effectBloomExposure = 1.0f;
    effectBloomIterations = 5;

    effectOitEnabled = true;
    effectGlowEnabled = true;
    effectFogEnabled = true;

    computeGroups = { 1, 1, 1 };

    //rootId    = SID("<root>");
    //volumeId  = SID("<volume>");
    //cubeMapId = SID("<cube_map>");
    //skyboxId  = SID("<skybox>");

    rootId = 1;
    volumeId = 2;
    cubeMapId = 3;
    skyboxId = 4;

    /////////////////////////
    // TEMPORARIES
}
