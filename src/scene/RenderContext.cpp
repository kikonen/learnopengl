#include "RenderContext.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ki/GL.h"
#include "component/Light.h"

#include "scene/Scene.h"
#include "scene/NodeRegistry.h"


RenderContext::RenderContext(
    const std::string& name,
    const Assets& assets,
    const RenderClock& clock,
    GLState& state,
    Scene* scene,
    Camera& camera,
    int width,
    int height)
    : name(name),
    assets(assets),
    clock(clock),
    state(state),
    scene(scene),
    registry(scene->registry),
    camera(camera),
    width(width),
    height(height)
{
    viewMatrix = camera.getView();

    aspectRatio = (float)width / (float)height;
    projectionMatrix = glm::perspective(
        glm::radians((float)camera.getZoom()),
        aspectRatio,
        assets.nearPlane,
        assets.farPlane);

    projectedMatrix = projectionMatrix * viewMatrix;

    for (int i = 0; i < CLIP_PLANE_COUNT; i++) {
        clipPlanes.clipping[i].enabled = false;
    }

    updateFrustum();
}

RenderContext::~RenderContext()
{
    if (assets.frustumDebug)
        KI_INFO_SB(name << ": draw: " << drawCount << " skip: " << skipCount);
}

void RenderContext::bindGlobal() const
{
    if (useWireframe) {
        state.polygonFrontAndBack(GL_LINE);
    }
    else {
        state.polygonFrontAndBack(GL_FILL);
    }
}

void RenderContext::bindUBOs() const
{
    // https://stackoverflow.com/questions/49798189/glbuffersubdata-offsets-for-structs
    bindMatricesUBO();
    bindDataUBO();
    bindClipPlanesUBO();
    bindLightsUBO();
}

void RenderContext::bindMatricesUBO() const
{
    MatricesUBO matricesUbo = { projectedMatrix, projectionMatrix, viewMatrix, lightSpaceMatrix };

    glNamedBufferSubData(scene->ubo.matrices, 0, sizeof(MatricesUBO), &matricesUbo);
}

void RenderContext::bindDataUBO() const
{
    DataUBO dataUbo{
        camera.getPos(),
        (float)clock.ts,
        assets.fogColor,
        assets.fogStart,
        assets.fogEnd,
    };

    glNamedBufferSubData(scene->ubo.data, 0, sizeof(DataUBO), &dataUbo);
}

void RenderContext::bindClipPlanesUBO() const
{
    glNamedBufferSubData(scene->ubo.clipPlanes, 0, sizeof(ClipPlanesUBO), &clipPlanes);
}

void RenderContext::bindLightsUBO() const
{
    LightsUBO lightsUbo;
    {
        auto& node = scene->registry.dirLight;
        if (node && useLight) {
            lightsUbo.light = node->light->toDirLightUBO();
            //lights.light.use = false;
        }
        else {
            DirLightUBO none;
            none.use = false;
            lightsUbo.light = none;
        }
    }

    {
        int index = 0;
        for (auto& node : scene->registry.pointLights) {
            if (!useLight) continue;
            if (index >= LIGHT_COUNT) break;
            if (!node->light->enabled) continue;

            lightsUbo.pointLights[index] = node->light->toPointightUBO();
            //lights.pointLights[index].use = false;
            index++;
        }
        PointLightUBO none;
        none.use = false;
        while (index < LIGHT_COUNT) {
            lightsUbo.pointLights[index] = none;
            index++;
        }
    }

    {
        int index = 0;
        for (auto& node : scene->registry.spotLights) {
            if (!useLight) continue;
            if (index >= LIGHT_COUNT) break;
            if (!node->light->enabled) continue;

            lightsUbo.spotLights[index] = node->light->toSpotLightUBO();
            //lights.spotLights[index].use = false;
            index++;
        }
        SpotLightUBO none;
        none.use = false;
        while (index < LIGHT_COUNT) {
            lightsUbo.spotLights[index] = none;
            index++;
        }
    }

    glNamedBufferSubData(scene->ubo.lights, 0, sizeof(LightsUBO), &lightsUbo);
}

void RenderContext::updateFrustum()
{
    // TODO KI https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
    // https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/entity.h

    const float fovY = glm::radians(camera.getZoom());
    const glm::vec3& pos = camera.getPos();
    const glm::vec3& front = camera.getViewFront();
    const glm::vec3& up = camera.getViewUp();
    const glm::vec3& right = camera.getViewRight();

    const float halfVSide = assets.farPlane * tanf(fovY * .5f);
    const float halfHSide = halfVSide * aspectRatio;
    const glm::vec3 frontMultFar = assets.farPlane * front;

    // NOTE KI near and far plane don't have camee pos as "point in plane"
    // NOTE KI other side HAVE camra pos as "point in plane"

    frustum.nearFace = {
        pos + assets.nearPlane * front,
        front };

    frustum.farFace = {
        pos + frontMultFar,
        -front };

    frustum.rightFace = {
        pos,
        glm::cross(up, frontMultFar + right * halfHSide) };

    frustum.leftFace = {
        pos,
        glm::cross(frontMultFar - right * halfHSide, up) };

    frustum.topFace = {
        pos,
        glm::cross(right, frontMultFar - up * halfVSide) };

    frustum.bottomFace = {
        pos,
        glm::cross(frontMultFar + up * halfVSide, right) };
}

