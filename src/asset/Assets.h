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


    int glfwSwapInterval;
    bool glDebug;

    glm::vec2 resolutionScale;

    bool asyncLoaderEnabled;
    int asyncLoaderDelay;

    std::string logFile;

    std::string modelsDir;
    std::string shadersDir;
    std::string spritesDir;
    std::string texturesDir;

    bool placeholderTextureAlways;
    std::string placeholderTexture;

    bool useIMGUI;
    bool useWrireframe;
    bool useLight;

    bool showNormals;
    bool showRearView;
    bool showShadowMapView;
    bool showReflectionView;
    bool showRefractionView;
    bool showObjectIDView;

    bool showSelectionWireframe;
    bool showVolume;

    bool rasterizerDiscard;

    float renderFrequency;
    bool debugClearColor;
    int clearColor;

    bool frustumEnabled;
    bool frustumDebug;

    // NOTE KI mirror does not tolerate much skip
    float mirrorRenderFrequency;

    int waterTileSize;
    // NOTE KI water tolerates less skip than shadow/cube
    // => i.e. it's "sharper" thus lack is more visible to user
    float waterRenderFrequency;

    int terrainVertexCount;
    int terrainTileSize;

    int batchSize;

    float nearPlane;
    float farPlane;

    glm::vec4 fogColor;
    float fogStart;
    float fogEnd;

    // NOTE KI MUST match lookup() in light shadow shader
    float shadowNearPlane;
    float shadowFarPlane;
    int shadowMapSize;
    float shadowRenderFrequency;

    int mirrorReflectionSize;
    int mirrorRefractionSize;

    int waterReflectionSize;
    int waterRefractionSize;

    int cubeMapSize;
    float cubeMapRenderFrequency;

    float cubeMapNearPlane;
    float cubeMapFarPlane;

    ViewportEffect viewportEffect;

    uuids::uuid volumeUUID;

    // NOTE KI TEMPORARY HACKS
    // => provide logic for these via scenefile
};
