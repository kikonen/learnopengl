#include "RenderContext.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ki/GL.h"
#include "component/Light.h"

#include "command/CommandEngine.h"
#include "command/ScriptEngine.h"

#include "scene/Scene.h"
#include "scene/NodeRegistry.h"


RenderContext::RenderContext(
    const std::string& name,
    const RenderContext* parent,
    Camera& camera,
    int width,
    int height)
    : RenderContext(
        name,
        parent,
        parent->assets,
        parent->clock,
        parent->state,
        parent->scene,
        camera,
        parent->nearPlane,
        parent->farPlane,
        width,
        height)
{}

RenderContext::RenderContext(
    const std::string& name,
    const RenderContext* parent,
    Camera& camera,
    float nearPlane,
    float farPlane,
    int width,
    int height)
    : RenderContext(
        name,
        parent,
        parent->assets,
        parent->clock,
        parent->state,
        parent->scene,
        camera,
        nearPlane,
        farPlane,
        width,
        height)
{}

RenderContext::RenderContext(
    const std::string& name,
    const RenderContext* parent,
    const Assets& assets,
    const RenderClock& clock,
    GLState& state,
    Scene* scene,
    Camera& camera,
    float nearPlane,
    float farPlane,
    int width,
    int height)
    : name(name),
    parent(parent),
    assets(assets),
    clock(clock),
    state(state),
    scene(scene),
    registry(scene->registry),
    commandEngine(scene->commandEngine),
    scriptEngine(scene->scriptEngine),
    camera(camera),
    nearPlane(nearPlane),
    farPlane(farPlane),
    resolution{ width, height },
    aspectRatio((float)width / (float)height)
{
    matrices.view = camera.getView();

    matrices.projection = glm::perspective(
        glm::radians((float)camera.getZoom()),
        aspectRatio,
        nearPlane,
        farPlane);

    matrices.projected = matrices.projection * matrices.view;

    for (int i = 0; i < CLIP_PLANE_COUNT; i++) {
        clipPlanes.clipping[i].enabled = false;
    }

    updateFrustum();
}

RenderContext::~RenderContext()
{
    if (parent) {
        parent->drawCount += drawCount;
        parent->skipCount += skipCount;
    }
    //if (assets.frustumDebug)
    //    KI_INFO_SB(name << ": draw: " << drawCount << " skip: " << skipCount);
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
    //MatricesUBO matricesUbo = {
    //    projectedMatrix,
    //    projectionMatrix,
    //    viewMatrix,
    //    lightProjectedMatrix,
    //    shadowMatrix,
    //};

    glNamedBufferSubData(scene->ubo.matrices, 0, sizeof(MatricesUBO), &matrices);
}

void RenderContext::bindDataUBO() const
{
    DataUBO dataUbo{
        camera.getPos(),
        (float)clock.ts,
        resolution,
        0,
        0,
        assets.fogColor,
        assets.fogStart,
        assets.fogEnd,
    };

    glNamedBufferSubData(scene->ubo.data, 0, sizeof(DataUBO), &dataUbo);
}

void RenderContext::bindClipPlanesUBO() const
{
    int count = 0;
    for (int i = 0; i < CLIP_PLANE_COUNT; i++) {
        if (!clipPlanes.clipping[i].enabled) continue;
        count++;
    }

    clipPlanes.clipCount = count;
    glNamedBufferSubData(scene->ubo.clipPlanes, 0, sizeof(ClipPlanesUBO), &clipPlanes);
}

void RenderContext::bindLightsUBO() const
{
    LightsUBO lightsUbo;
    if (!useLight) {
        lightsUbo.dirCount = 0;
        lightsUbo.pointCount = 0;
        lightsUbo.spotCount = 0;
    }

    if (useLight) {
        auto& node = scene->registry.m_dirLight;
        if (node) {
            lightsUbo.dir[0] = node->m_light->toDirLightUBO();
            lightsUbo.dirCount = 1;
        }
        else {
            lightsUbo.dirCount = 0;
        }
    }

    if (useLight) {
        int count = 0;
        for (auto& node : scene->registry.m_pointLights) {
            if (count >= LIGHT_COUNT) break;
            if (!node->m_light->enabled) continue;

            lightsUbo.pointLights[count] = node->m_light->toPointightUBO();
            count++;
        }
        lightsUbo.pointCount = count;
    }

    if (useLight) {
        int count = 0;
        for (auto& node : scene->registry.m_spotLights) {
            if (count>= LIGHT_COUNT) break;
            if (!node->m_light->enabled) continue;

            lightsUbo.spotLights[count] = node->m_light->toSpotLightUBO();
            count++;
        }
        lightsUbo.spotCount = count;
    }

    glNamedBufferSubData(scene->ubo.lights, 0, sizeof(LightsUBO), &lightsUbo);
}

void RenderContext::updateFrustum()
{
    // TODO KI https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
    // https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/entity.h

    // NOTE KI use 90 angle for culling; smaller does cut-off too early
    // => 90 angle neither working correctly always for terrain tiles
    // => TODO KI WHAT is failing
    const float fovY = glm::radians(camera.getZoom());
    //const float fovY = glm::radians(90.f);
    const glm::vec3& pos = camera.getPos();
    const glm::vec3& front = camera.getViewFront();
    const glm::vec3& up = camera.getViewUp();
    const glm::vec3& right = camera.getViewRight();

    const float halfVSide = farPlane * tanf(fovY * .5f);
    const float halfHSide = halfVSide * aspectRatio;
    const glm::vec3 frontMultFar = farPlane * front;

    // NOTE KI near and far plane don't have camee pos as "point in plane"
    // NOTE KI other side HAVE camra pos as "point in plane"

    frustum.nearFace = {
        pos + nearPlane * front,
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

