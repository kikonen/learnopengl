#include "RenderContext.h"
#include "RenderContext.h"

#include <glm/glm.hpp>
//#include <glm/ext.hpp>

#include "ki/GL.h"
#include "component/Light.h"

#include "asset/ImageTexture.h"

#include "command/CommandEngine.h"
#include "command/ScriptEngine.h"

#include "backend/RenderSystem.h"

#include "registry/NodeRegistry.h"
#include "registry/EntityRegistry.h"

#include "scene/Scene.h"
#include "scene/RenderData.h"
#include "scene/Batch.h"

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
        parent->m_clock,
        parent->state,
        parent->m_scene,
        camera,
        parent->m_backend,
        parent->m_nearPlane,
        parent->m_farPlane,
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
        parent->m_clock,
        parent->state,
        parent->m_scene,
        camera,
        parent->m_backend,
        nearPlane,
        farPlane,
        width,
        height)
{}

RenderContext::RenderContext(
    const std::string& name,
    const RenderContext* parent,
    const Assets& assets,
    const ki::RenderClock& clock,
    GLState& state,
    Scene* scene,
    Camera& camera,
    backend::RenderSystem* backend,
    float nearPlane,
    float farPlane,
    int width,
    int height)
    : m_name(name),
    m_parent(parent),
    assets(assets),
    m_clock(clock),
    state(state),
    m_scene(scene),
    m_batch(*scene->m_batch),
    m_nodeRegistry(*scene->m_nodeRegistry),
    m_entityRegistry(*scene->m_entityRegistry),
    commandEngine(*scene->m_commandEngine),
    scriptEngine(*scene->m_scriptEngine),
    m_camera(camera),
    m_backend(backend),
    m_nearPlane(nearPlane),
    m_farPlane(farPlane),
    m_resolution{ width, height },
    m_aspectRatio((float)width / (float)height)
{
    if (parent) {
        m_useWireframe = m_parent->m_useWireframe;
        m_useLight = m_parent->m_useLight;
        m_useBlend = m_parent->m_useBlend;
        m_useFrustum = m_parent->m_useFrustum;
    }

    m_camera.setupProjection(
        m_aspectRatio,
        m_nearPlane,
        m_farPlane);

    m_matrices.view = m_camera.getView();
    m_matrices.projection = m_camera.getProjection();
    m_matrices.projected = m_camera.getProjected();

    if (false) {
        const auto& view = m_matrices.view;

        glm::mat4 inverse = glm::inverse(view);
        glm::mat4 transpose = glm::transpose(view);

        glm::vec3 right{ view[0][0], view[1][0], view[2][0] };
        glm::vec3 up{ view[0][1], view[1][1], view[2][1] };
        glm::vec3 pos{ inverse[3] };

        const auto& cameraUp = m_camera.getViewUp();
        const auto& cameraRight = m_camera.getViewRight();
        const auto& cameraPos = m_camera.getPos();
        int x = 0;
    }

    m_data = {
        m_camera.getPos(),
        0,
        m_camera.getFront(),
        0,
        m_camera.getViewUp(),
        0,
        m_camera.getViewRight(),
        (float)m_clock.ts,
        m_resolution,
        0,
        0,
        assets.fogColor,
        assets.fogStart,
        assets.fogEnd,
        };

    for (int i = 0; i < CLIP_PLANE_COUNT; i++) {
        m_clipPlanes.clipping[i].enabled = false;
    }
}

RenderContext::~RenderContext()
{
    if (m_parent) {
        m_parent->m_drawCount += m_drawCount;
        m_parent->m_skipCount += m_skipCount;
    }

    if (false && assets.frustumDebug) {
        KI_INFO(fmt::format(
            "CTX: name={}, draw=, skip={}",
            m_name, m_drawCount, m_skipCount));
    }
}

void RenderContext::bindDefaults() const
{
    // https://cmichel.io/understanding-front-faces-winding-order-and-normals
    state.enable(GL_CULL_FACE);
    state.cullFace(GL_BACK);
    state.frontFace(GL_CCW);

    state.polygonFrontAndBack(GL_FILL);
    state.disable(GL_BLEND);
}

//void RenderContext::bindDraw(
//    bool renderBack,
//    bool wireframe) const
//{
//    if (renderBack) {
//        state.disable(GL_CULL_FACE);
//    }
//    else {
//        state.enable(GL_CULL_FACE);
//    }
//
//    if (wireframe) {
//        state.polygonFrontAndBack(GL_LINE);
//    }
//    else {
//        state.polygonFrontAndBack(GL_FILL);
//    }
//}

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
    m_scene->m_renderData->updateMatrices(m_matrices);
}

void RenderContext::bindDataUBO() const
{
    m_scene->m_renderData->updateData(m_data);
}

void RenderContext::bindClipPlanesUBO() const
{
    m_scene->m_renderData->updateClipPlanes(m_clipPlanes);
}

void RenderContext::bindLightsUBO() const
{
    auto& nodeRegistry = m_nodeRegistry;

    LightsUBO lightsUbo{};
    if (!m_useLight) {
        lightsUbo.dirCount = 0;
        lightsUbo.pointCount = 0;
        lightsUbo.spotCount = 0;
    }

    if (m_useLight) {
        auto& node = nodeRegistry.m_dirLight;
        if (node && node->m_light->m_enabled) {
            lightsUbo.dir[0] = node->m_light->toDirLightUBO();
            lightsUbo.dirCount = 1;
        }
        else {
            lightsUbo.dirCount = 0;
        }
    }

    if (m_useLight) {
        int count = 0;
        for (auto& node : nodeRegistry.m_pointLights) {
            if (count >= LIGHT_COUNT) break;
            if (!node->m_light->m_enabled) continue;

            lightsUbo.pointLights[count] = node->m_light->toPointightUBO();
            count++;
        }
        lightsUbo.pointCount = count;
    }

    if (m_useLight) {
        int count = 0;
        for (auto& node : nodeRegistry.m_spotLights) {
            if (count>= LIGHT_COUNT) break;
            if (!node->m_light->m_enabled) continue;

            lightsUbo.spotLights[count] = node->m_light->toSpotLightUBO();
            count++;
        }
        lightsUbo.spotCount = count;
    }

    m_scene->m_renderData->updateLights(lightsUbo);
}

const FrustumNew& RenderContext::getFrustumNew() const
{
    if (assets.frustumEnabled && m_useFrustum && !m_frustumNewPrepared) {
        updateFrustumNew(m_frustumNew, m_matrices.view, true);
        m_frustumNewPrepared = true;
    }
    return m_frustumNew;
}


// Fast Extraction of Viewing Frustum Planes from the World- View-Projection Matrix
// http://gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
// https://www.reddit.com/r/gamedev/comments/5zatbm/frustum_culling_in_opengl_glew_c/
// https://donw.io/post/frustum-point-extraction/
// https://iquilezles.org/articles/frustum/
// https://gist.github.com/podgorskiy/e698d18879588ada9014768e3e82a644
// https://stackoverflow.com/questions/8115352/glmperspective-explanation
void RenderContext::updateFrustumNew(
    FrustumNew& frustum,
    const glm::mat4& mat,
    bool normalize) const
{
    // Left clipping plane
    frustum.m_planes[0] = mat[3] + mat[0];

    // Right clipping plane
    frustum.m_planes[1] = mat[3] - mat[0];

    // Top clipping plane
    frustum.m_planes[2] = mat[3] - mat[1];

    // Bottom clipping plane
    frustum.m_planes[3] = mat[3] + mat[1];

    // Near clipping plane
    frustum.m_planes[4] = mat[3] + mat[2];

    // Far clipping plane
    frustum.m_planes[5] = mat[3] - mat[2];

    if (normalize) {
        frustum.normalize();
    }
}
