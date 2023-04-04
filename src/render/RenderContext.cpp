#include "RenderContext.h"
#include "RenderContext.h"

#include <glm/glm.hpp>
//#include <glm/ext.hpp>

#include "ki/GL.h"
#include "component/Light.h"

#include "asset/ImageTexture.h"

#include "component/Camera.h"

#include "command/CommandEngine.h"
#include "command/ScriptEngine.h"

#include "backend/RenderSystem.h"

#include "registry/NodeRegistry.h"
#include "registry/EntityRegistry.h"

#include "render/NodeDraw.h"
#include "render/RenderData.h"
#include "render/FrameBuffer.h"
#include "render/Batch.h"


RenderContext::RenderContext(
    const std::string& name,
    const RenderContext* parent,
    Camera* camera,
    int width,
    int height)
    : RenderContext(
        name,
        parent,
        parent->m_clock,
        parent->m_assets,
        parent->m_commandEngine,
        parent->m_scriptEngine,
        parent->m_registry,
        parent->m_renderData,
        parent->m_nodeDraw,
        parent->m_batch,
        parent->m_state,
        camera,
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
        parent->m_clock,
        parent->m_assets,
        parent->m_commandEngine,
        parent->m_scriptEngine,
        parent->m_registry,
        parent->m_renderData,
        parent->m_nodeDraw,
        parent->m_batch,
        parent->m_state,
        camera,
        nearPlane,
        farPlane,
        width,
        height)
{}

RenderContext::RenderContext(
    const std::string& name,
    const RenderContext* parent,
    const ki::RenderClock& clock,
    const Assets& assets,
    CommandEngine* commandEngine,
    ScriptEngine* scriptEngine,
    Registry* registry,
    RenderData* renderData,
    NodeDraw* nodeDraw,
    Batch* batch,
    GLState& state,
    Camera* camera,
    float nearPlane,
    float farPlane,
    int width,
    int height)
    : m_name(name),
    m_parent(parent),
    m_assets(assets),
    m_clock(clock),
    m_state(state),
    m_renderData(renderData),
    m_nodeDraw(nodeDraw),
    m_batch(batch),
    m_registry(registry),
    m_commandEngine(commandEngine),
    m_scriptEngine(scriptEngine),
    m_camera(camera),
    m_nearPlane(nearPlane),
    m_farPlane(farPlane),
    m_resolution({ width, height }),
    m_aspectRatio((float)width / (float)height)
{
    if (m_parent) {
        m_forceWireframe = m_parent->m_forceWireframe;
        m_useLight = m_parent->m_useLight;
        m_allowBlend = m_parent->m_allowBlend;
    }

    m_camera->setupProjection(
        m_aspectRatio,
        m_nearPlane,
        m_farPlane);

    m_matrices.u_view = m_camera->getView();

    m_matrices.u_projection = m_camera->getProjection();
    m_matrices.u_projected = m_camera->getProjected();

    {
        // https://www.rioki.org/2013/03/07/glsl-skybox.html
        // NOTE KI remove translation from the view matrix for skybox
        glm::mat4 m = m_matrices.u_view;
        m[3][0] = 0.f;
        m[3][1] = 0.f;
        m[3][2] = 0.f;

        m_matrices.u_viewSkybox = glm::inverse(m) * glm::inverse(m_matrices.u_projection);
    }

    m_data = {
        m_camera->getWorldPosition(),
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
        assets.fogRatio,
        true,
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
    m_state.setEnabled(GL_CULL_FACE, true);
    m_state.cullFace(GL_BACK);
    m_state.frontFace(GL_CCW);

    m_state.polygonFrontAndBack(GL_FILL);
    m_state.setEnabled(GL_BLEND, false);
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
    m_renderData->updateMatrices(m_matrices);
}

void RenderContext::updateDataUBO() const
{
    m_renderData->updateData(m_data);
}

void RenderContext::updateClipPlanesUBO() const
{
    m_renderData->updateClipPlanes(m_clipPlanes);
}

void RenderContext::updateLightsUBO() const
{
    m_renderData->updateLights(m_registry, m_useLight);
}
