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

    glm::vec2 resolutionScale;

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
    bool frustumDebug;

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

    int terrainVertexCount;
    int terrainTileSize;

    int batchSize;
    int batchBuffers;

    float nearPlane;
    float farPlane;

    glm::vec4 fogColor;
    float fogStart;
    float fogEnd;

    // NOTE KI MUST match lookup() in light shadow shader
    float shadowNearPlane;
    float shadowFarPlane;
    float shadowFrustumSize;
    int shadowMapSize;
    int shadowRenderFrameStart;
    int shadowRenderFrameStep;

    int mirrorReflectionSize;
    int mirrorRefractionSize;

    int waterReflectionSize;
    int waterRefractionSize;

    int cubeMapSize;
    int cubeMapRenderFrameStart;
    int cubeMapRenderFrameStep;

    float cubeMapNearPlane;
    float cubeMapFarPlane;

    ViewportEffect viewportEffect;

    uuids::uuid rootUUID;
    uuids::uuid volumeUUID;
    uuids::uuid cubeMapUUID;

    // NOTE KI TEMPORARY HACKS
    // => provide logic for these via scenefile
};
