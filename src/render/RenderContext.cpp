#include "RenderContext.h"
#include "RenderContext.h"

//#include <glm/glm.hpp>
//#include <glm/ext.hpp>

#include <fmt/format.h>

#include "util/Log.h"

#include "kigl/GLState.h"

#include "asset/Assets.h"

#include "component/Camera.h"

#include "registry/Registry.h"

#include "render/NodeDraw.h"
#include "render/RenderData.h"
#include "render/FrameBuffer.h"
#include "render/Batch.h"
#include "render/DebugContext.h"


RenderContext::RenderContext(
    std::string_view name,
    const RenderContext* parent,
    Camera* camera,
    int width,
    int height)
    : RenderContext(
        name,
        parent,
        parent->m_clock,
        parent->m_registry,
        parent->m_renderData,
        parent->m_nodeDraw,
        parent->m_batch,
        camera,
        parent->m_nearPlane,
        parent->m_farPlane,
        width,
        height,
        parent->m_debugContext)
{}

RenderContext::RenderContext(
    std::string_view name,
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
        parent->m_registry,
        parent->m_renderData,
        parent->m_nodeDraw,
        parent->m_batch,
        camera,
        nearPlane,
        farPlane,
        width,
        height,
        parent->m_debugContext)
{}

RenderContext::RenderContext(
    std::string_view name,
    const RenderContext* parent,
    const ki::RenderClock& clock,
    Registry* registry,
    render::RenderData* renderData,
    render::NodeDraw* nodeDraw,
    render::Batch* batch,
    Camera* camera,
    float nearPlane,
    float farPlane,
    int width,
    int height,
    const render::DebugContext* const debugContext)
    : m_name{ name },
    m_parent{ parent },
    m_assets{ Assets::get() },
    m_state{ m_parent ? m_parent->m_state : kigl::GLState::get() },
    m_clock{ clock },
    m_renderData{ renderData },
    m_nodeDraw{ nodeDraw },
    m_batch{ batch },
    m_registry{ registry },
    m_camera{ camera },
    m_nearPlane{ nearPlane },
    m_farPlane{ farPlane },
    m_resolution({ width, height }),
    m_aspectRatio{ (float)width / (float)height },
    m_depthFunc{ GL_LESS },
    m_debugContext{ debugContext }
{
    auto& assets = m_assets;

    if (m_parent) {
        m_useLight = m_parent->m_useLight;
        m_shadow = m_parent->m_shadow;

        m_forceSolid = m_parent->m_forceSolid;
        m_forceWireframe = m_parent->m_forceWireframe;

        m_allowDrawDebug = m_parent->m_allowDrawDebug;
    }

    m_camera->setupProjection(
        m_aspectRatio,
        m_nearPlane,
        m_farPlane);

    m_matrices.u_view = m_camera->getView();
    m_matrices.u_invView = glm::inverse(m_matrices.u_view);

    m_matrices.u_projection = m_camera->getProjection();
    m_matrices.u_invProjection = glm::inverse(m_matrices.u_projection);

    m_matrices.u_projected = m_camera->getProjected();

    m_matrices.u_mainProjected = m_parent ? m_parent->m_camera->getProjected() : m_camera->getProjected();

    {
        // https://www.rioki.org/2013/03/07/glsl-skybox.html
        // NOTE KI remove translation from the view matrix for skybox
        glm::mat4 m = m_matrices.u_view;
        m[3][0] = 0.f;
        m[3][1] = 0.f;
        m[3][2] = 0.f;

        m_matrices.u_viewSkybox = glm::inverse(m) * glm::inverse(m_matrices.u_projection);

        const auto& planes = m_camera->getFrustumPlanes();
        std::copy(
            std::begin(planes),
            std::end(planes),
            std::begin(m_matrices.u_frustumPlanes));
    }

    m_data = {
        m_camera->getWorldPosition(),
        //0,
        m_camera->getViewFront(),
        //0,
        m_camera->getViewUp(),
        //0,
        m_camera->getViewRight(),
        //0,
        m_parent ? m_parent->m_camera->getWorldPosition() : m_camera->getWorldPosition(),
        //0,
        m_parent ? m_parent->m_camera->getViewFront() : m_camera->getViewFront(),
        //0,
        m_parent ? m_parent->m_camera->getViewUp() : m_camera->getViewUp(),
        //0,
        m_parent ? m_parent->m_camera->getViewRight() : m_camera->getViewRight(),
        //0,
        assets.fogColor,
        // NOTE KI keep original screen resolution across the board
        // => current buffer resolution is separately in bufferInfo UBO
        m_parent ? m_parent->m_resolution : m_resolution,

        assets.cubeMapEnabled,
        assets.skyboxEnabled,

        assets.environmentMapEnabled,

        assets.shadowVisual,

        assets.fogStart,
        assets.fogEnd,
        assets.fogDensity,

        assets.hdrGamma,
        assets.hdrExposure,
        // NOTE KI u_shadowPlanes not initialized
        assets.effectBloomExposure,

        static_cast<float>(m_clock.ts),
        0, // shadowCount
    };

    if (m_debugContext) {
        m_debug = {
            m_debugContext->m_entityId,
            m_debugContext->m_animationBoneIndex,
            m_debugContext->m_animationDebugBoneWeight,
            m_debugContext->m_parallaxDepth,
            m_debugContext->m_parallaxMethod,
        };
    }
    else {
        m_debug = {
            0,
            0,
            false,
        };
    }

    //KI_INFO_OUT(fmt::format("ts: {}", m_data.u_time));

    m_clipPlanes.u_clipCount = 0;
}

RenderContext::~RenderContext()
{
}

void RenderContext::bindDefaults() const
{
    validateRender("bind_defaults");

    auto& state = m_state;

    // https://cmichel.io/understanding-front-faces-winding-order-and-normals
    state.setEnabled(GL_CULL_FACE, m_defaults.m_cullFaceEnabled);
    state.cullFace(m_defaults.m_cullFace);
    state.frontFace(m_defaults.m_frontFace);

    state.polygonFrontAndBack(m_defaults.m_polygonFrontAndBack);
    state.setEnabled(GL_BLEND, m_defaults.m_blendEnabled);
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
    validateRender("update_matrices_ubo");
    m_renderData->updateMatrices(m_matrices);
}

void RenderContext::updateDataUBO() const
{
    validateRender("update_data_ubo");
    m_renderData->updateData(m_data);
    m_renderData->updateDebug(m_debug);
}

void RenderContext::updateDebugUBO() const
{
    validateRender("update_debug_ubo");
    m_renderData->updateDebug(m_debug);
}

void RenderContext::updateClipPlanesUBO() const
{
    validateRender("update_clip_planes_ubo");
    m_renderData->updateClipPlanes(m_clipPlanes);
}

void RenderContext::updateLightsUBO() const
{
    validateRender("update_lights_ubo");
    m_renderData->updateLights(m_registry, m_useLight);
}

void RenderContext::validateRender(std::string_view label) const
{
    if (!m_batch->isFlushed()) {
        throw std::runtime_error{ fmt::format("CONTEXT: Batch was NOT flushed: name={}, label={}", m_name, label)};
    }

    //int fbo = m_state.getFrameBuffer();
    //if (fbo > 0) {
    //    throw std::runtime_error{ fmt::format("CONTEXT: Stale frame buffer: context={}, fbo={}, label={}", m_name, fbo, label)};
    //}
}

void RenderContext::copyShadowFrom(const RenderContext& b)
{
    std::copy(
        std::begin(b.m_matrices.u_shadow),
        std::end(b.m_matrices.u_shadow),
        std::begin(m_matrices.u_shadow));

    //std::copy(
    //    std::begin(b.m_matrices.u_shadowProjected),
    //    std::end(b.m_matrices.u_shadowProjected),
    //    std::begin(m_matrices.u_shadowProjected));
}

UpdateContext RenderContext::toUpdateContext() const
{
    return {
        m_clock,
        m_registry,
    };
}

PrepareContext RenderContext::toPrepareContext() const
{
    return {
        m_registry,
    };
}
