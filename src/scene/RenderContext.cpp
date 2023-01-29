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
    Camera* camera,
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
    Camera* camera,
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
    Camera* camera,
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
    m_batch(scene->m_batch.get()),
    m_registry(scene->m_registry.get()),
    m_commandEngine(scene->m_commandEngine.get()),
    m_scriptEngine(scene->m_scriptEngine.get()),
    m_camera(camera),
    m_backend(backend),
    m_nearPlane(nearPlane),
    m_farPlane(farPlane),
    m_resolution{ width, height },
    m_aspectRatio((float)width / (float)height)
{
    if (parent) {
        m_forceWireframe = m_parent->m_forceWireframe;
        m_useLight = m_parent->m_useLight;
        m_allowBlend = m_parent->m_allowBlend;
    }

    m_camera->setupProjection(
        m_aspectRatio,
        m_nearPlane,
        m_farPlane);

    m_matrices.view = m_camera->getView();
    // NOTE KI remove translation from the view matrix for skybox
    m_matrices.viewSkybox = glm::mat4(glm::mat3(m_matrices.view));
    m_matrices.projection = m_camera->getProjection();
    m_matrices.projected = m_camera->getProjected();

    m_data = {
        m_camera->getViewPosition(),
        0,
        m_camera->getViewFront(),
        0,
        m_camera->getViewUp(),
        0,
        m_camera->getViewRight(),
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

void RenderContext::updateUBOs() const
{
    // https://stackoverflow.com/questions/49798189/glbuffersubdata-offsets-for-structs
    updateMatricesUBO();
    updateDataUBO();
    updateClipPlanesUBO();
    updateLightsUBO();
}

void RenderContext::updateMatricesUBO() const
{
    m_scene->m_renderData->updateMatrices(m_matrices);
}

void RenderContext::updateDataUBO() const
{
    m_scene->m_renderData->updateData(m_data);
}

void RenderContext::updateClipPlanesUBO() const
{
    m_scene->m_renderData->updateClipPlanes(m_clipPlanes);
}

void RenderContext::updateLightsUBO() const
{
    m_scene->m_renderData->updateLights(m_registry, m_useLight);
}
