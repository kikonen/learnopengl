#pragma once

#include <string>
#include <vector>

#include "glm/glm.hpp"

#include "ki/size.h"

#include "ViewportEffect.h"


// configure assets locations
class Assets final
{
public:
    static const Assets& get() noexcept;
    static Assets& modify() noexcept;
    static void set(const Assets& assets) noexcept;

    Assets();

public:
    int glsl_version[3];
    std::string glsl_version_str;

    bool glVendorNvidia;
    bool glVendorIntel;

    int glPreferredTextureFormatRGB;
    int glPreferredTextureFormatRGBA;

    int glfwSwapInterval;

    // https://www.glfw.org/docs/3.3/window_guide.html#window_hints

    // https://learnopengl.com/in-practice/debugging
    bool glDebug;

    // https://www.khronos.org/opengl/wiki/OpenGL_Error#No_error_contexts
    bool glNoError;

    // https://www.khronos.org/opengl/wiki/Synchronization
    bool glUseMapped;
    bool glUseInvalidate;
    bool glUseFence;
    bool glUseSingleFence;
    bool glUseDebugFence;
    bool glUseFinish;

    bool glslUseDebug;

    bool compressedTexturesEnabled;
    bool prepassDepthEnabled;

    float gBufferScale;
    float waterReflectionBufferScale;
    float waterRefractionBufferScale;
    float mirrorReflectionBufferScale;

    glm::uvec2 windowSize;
    bool windowMaximized;
    bool windowFullScreen;

    glm::vec3 cameraMoveNormal;
    glm::vec3 cameraMoveRun;
    glm::vec3 cameraRotateNormal;
    glm::vec3 cameraRotateRun;
    glm::vec3 cameraZoomNormal;
    glm::vec3 cameraZoomRun;
    glm::vec3 cameraMouseSensitivity;

    bool asyncLoaderEnabled;
    int asyncLoaderDelay;

    bool useAssimpLoader;

    std::string logFile;

    std::string sceneDir;
    std::string sceneFile;

    std::string rootDir;

    std::string assetsDir;
    std::string modelsDir;
    std::string texturesDir;
    std::string fontsDir;

    std::string shadersDir;

    bool placeholderTextureAlways;
    std::string placeholderTexture;

    bool useImGui;
    bool imGuiDemo;
    float imGuiFontSize;
    std::string imGuiFontPath;

    bool useScript;
    bool useLight;

    bool forceWireframe;

    bool showNormals;
    bool showRearView;
    bool showShadowMapView;
    bool showReflectionView;
    bool showRefractionView;
    bool showObjectIDView;

    bool showVolume;
    bool showSelectionVolume;

    bool showHighlight;
    bool showSelection;
    bool showCubeMapCenter;
    bool showTagged;

    bool rasterizerDiscard;

    bool useDebugColor;

    bool useLodDebug;

    bool frustumEnabled;
    bool frustumCPU;
    bool frustumGPU;
    bool frustumAny;
    int frustumParallelLimit;
    bool frustumDebug;
    float cameraFov;

    int renderFrameStart;
    int renderFrameStep;

    int nodeRenderFrameStart;
    int nodeRenderFrameStep;

    // NOTE KI mirror does not tolerate much skip
    bool mirrorMapEnabled;
    int mirrorReflectionSize;
    float mirrorFov;

    bool mirrorRenderMirror;
    bool mirrorRenderWater;

    int mirrorRenderFrameStart;
    int mirrorRenderFrameStep;

    float mirrorMapNearPlane;
    float mirrorMapFarPlane;

    bool waterMapEnabled;
    int waterTileSize;
    // NOTE KI water tolerates less skip than shadow/cube
    // => i.e. it's "sharper" thus lack is more visible to user
    int waterRenderFrameStart;
    int waterRenderFrameStep;

    float waterMapNearPlane;
    float waterMapFarPlane;

    int terrainGridSize;

    int batchSize;
    int batchBuffers;
    bool batchDebug;

    bool drawDebug;

    float nearPlane;
    float farPlane;

    glm::vec4 fogColor;
    float fogStart;
    float fogEnd;
    float fogDensity;

    bool animationEnabled;
    bool animationJointTree;
    bool animationFirstFrameOnly;
    bool animationOnceOnly;
    int animationMaxCount;

    bool particleEnabled;
    int particleMaxCount;

    bool shadowMapEnabled;

    bool shadowPolygonOffsetEnabled;
    glm::vec2 shadowPolygonOffset;

    // NOTE KI MUST match lookup() in light shadow shader
    std::vector<float> shadowPlanes;
    std::vector<int> shadowMapSizes;

    int shadowRenderFrameStart;
    int shadowRenderFrameStep;

    bool shadowVisual;

    bool cubeMapEnabled;
    bool cubeMapSeamless;
    bool cubeMapSkipOthers;
    int cubeMapSize;

    bool cubeMapRenderMirror;
    bool cubeMapRenderWater;

    int cubeMapRenderFrameStart;
    int cubeMapRenderFrameStep;

    float cubeMapNearPlane;
    float cubeMapFarPlane;

    bool skyboxEnabled;
    int skyboxSize;

    bool environmentMapEnabled;
    int environmentMapSize;
    int irradianceMapSize;
    int prefilterMapSize;
    int brdfLutSize;

    bool viewportEffectEnabled;
    ViewportEffect viewportEffect;

    float hdrGamma;
    float hdrExposure;

    bool effectBloomEnabled;
    float effectBloomExposure;
    int effectBloomIterations;

    bool effectOitEnabled;
    bool effectGlowEnabled;
    bool effectFogEnabled;

    glm::uvec3 computeGroups;

    ki::node_id rootId;
    ki::node_id volumeId;
    ki::node_id cubeMapId;
    ki::node_id skyboxId;

    // NOTE KI TEMPORARY HACKS
    // => provide logic for these via scenefile
};
