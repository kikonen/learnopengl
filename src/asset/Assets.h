#pragma once

#include <string>

#include "glm/glm.hpp"
#include <stduuid/uuid.h>

enum class ViewportEffect {
    none = 0,
    invert = 1,
    grayScale = 2,
    sharpen = 3,
    blur = 4,
    edge = 5,
};

// configure assets locations
class Assets final
{
public:
    Assets();

public:
    int glsl_version[3];
    std::string glsl_version_str;

    int glPreferredTextureFormatRGB;
    int glPreferredTextureFormatRGBA;

    int glfwSwapInterval;

    // https://www.glfw.org/docs/3.3/window_guide.html#window_hints

    // https://learnopengl.com/in-practice/debugging
    bool glDebug;

    // https://www.khronos.org/opengl/wiki/OpenGL_Error#No_error_contexts
    bool glNoError;

    float resolutionScale;
    float gbufferScale;
    float bufferScale;

    glm::vec3 cameraMoveNormal;
    glm::vec3 cameraMoveRun;
    glm::vec3 cameraRotateNormal;
    glm::vec3 cameraRotateRun;
    glm::vec3 cameraZoomNormal;
    glm::vec3 cameraZoomRun;
    glm::vec3 cameraMouseSensitivity;

    bool asyncLoaderEnabled;
    int asyncLoaderDelay;

    std::string logFile;
    std::string sceneFile;

    std::string modelsDir;
    std::string shadersDir;
    std::string spritesDir;
    std::string texturesDir;

    bool placeholderTextureAlways;
    std::string placeholderTexture;

    bool useIMGUI;

    bool useScript;
    bool useLight;

    bool forceWireframe;

    bool renderCubeMap;
    bool renderShadowMap;
    bool renderMirrorMap;
    bool renderWaterMap;

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

    bool debugClearColor;
    int clearColor;

    bool frustumEnabled;
    bool frustumCPU;
    bool frustumGPU;
    bool frustumAny;
    bool frustumDebug;
    bool frustumVisual;

    float cameraFov;

    int renderFrameStart;
    int renderFrameStep;

    int nodeRenderFrameStart;
    int nodeRenderFrameStep;

    // NOTE KI mirror does not tolerate much skip
    int mirrorRenderFrameStart;
    int mirrorRenderFrameStep;
    float mirrorFov;

    int waterTileSize;
    // NOTE KI water tolerates less skip than shadow/cube
    // => i.e. it's "sharper" thus lack is more visible to user
    int waterRenderFrameStart;
    int waterRenderFrameStep;

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

    bool shadowPolygonOffsetEnabled;
    glm::vec2 shadowPolygonOffset;

    // NOTE KI MUST match lookup() in light shadow shader
    std::vector<float> shadowPlanes;
    std::vector<int> shadowMapSizes;
    int shadowRenderFrameStart;
    int shadowRenderFrameStep;

    int mirrorReflectionSize;
    int mirrorRefractionSize;

    int waterReflectionSize;
    int waterRefractionSize;

    bool cubeMapSeamless;

    int cubeMapSize;
    int cubeMapRenderFrameStart;
    int cubeMapRenderFrameStep;

    float cubeMapNearPlane;
    float cubeMapFarPlane;

    bool viewportEffectEnabled;
    ViewportEffect viewportEffect;

    glm::uvec3 computeGroups;

    uuids::uuid rootUUID;
    uuids::uuid volumeUUID;
    uuids::uuid cubeMapUUID;
    uuids::uuid skyboxUUID;

    // NOTE KI TEMPORARY HACKS
    // => provide logic for these via scenefile
};
