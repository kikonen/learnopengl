#include "RenderContext.h"
#include "RenderContext.h"

//#include <glm/glm.hpp>
//#include <glm/ext.hpp>

#include <fmt/format.h>

#include "util/Log.h"
#include "util/glm_util.h"
#include "util/glm_format.h"

#include "kigl/GLState.h"

#include "asset/Assets.h"


#include "registry/Registry.h"
#include "registry/SelectionRegistry.h"

#include "render/Camera.h"
//#include "render/NodeDraw.h"
#include "render/RenderData.h"
#include "render/FrameBuffer.h"
#include "render/Batch.h"
#include "render/DebugContext.h"


RenderContext::RenderContext(
    std::string_view name,
    const RenderContext* parent)
    : RenderContext(
        name,
        parent,
        parent->m_clock,
        parent->m_registry,
        parent->m_collection,
        parent->m_renderData,
        //parent->m_nodeDraw,
        parent->m_batch,
        parent->m_camera,
        parent->m_nearPlane,
        parent->m_farPlane,
        parent->m_resolution.x,
        parent->m_resolution.y,
        parent->m_dbg)
{}

RenderContext::RenderContext(
    std::string_view name,
    const RenderContext* parent,
    render::Camera* camera,
    int width,
    int height)
    : RenderContext(
        name,
        parent,
        parent->m_clock,
        parent->m_registry,
        parent->m_collection,
        parent->m_renderData,
        //parent->m_nodeDraw,
        parent->m_batch,
        camera,
        parent->m_nearPlane,
        parent->m_farPlane,
        width,
        height,
        parent->m_dbg)
{}

RenderContext::RenderContext(
    std::string_view name,
    const RenderContext* parent,
    render::Camera* camera,
    float nearPlane,
    float farPlane,
    int width,
    int height)
    : RenderContext(
        name,
        parent,
        parent->m_clock,
        parent->m_registry,
        parent->m_collection,
        parent->m_renderData,
        //parent->m_nodeDraw,
        parent->m_batch,
        camera,
        nearPlane,
        farPlane,
        width,
        height,
        parent->m_dbg)
{}

RenderContext::RenderContext(
    std::string_view name,
    const RenderContext* parent,
    const ki::RenderClock& clock,
    Registry* registry,
    render::NodeCollection* collection,
    render::RenderData* renderData,
    //render::NodeDraw* nodeDraw,
    render::Batch* batch,
    render::Camera* camera,
    float nearPlane,
    float farPlane,
    int width,
    int height,
    const render::DebugContext* const dbg)
    : m_name{ name },
    m_parent{ parent },
    m_assets{ Assets::get() },
    m_state{ m_parent ? m_parent->m_state : kigl::GLState::get() },
    m_clock{ clock },
    m_collection{ collection },
    m_renderData{ renderData },
    //m_nodeDraw{ nodeDraw },
    m_batch{ batch },
    m_registry{ registry },
    m_camera{ camera },
    m_nearPlane{ nearPlane },
    m_farPlane{ farPlane },
    m_resolution({ width, height }),
    m_aspectRatio{ (float)width / (float)height },
    m_depthFunc{ GL_LESS },
    m_dbg{ dbg }
{
    auto& assets = m_assets;

    if (m_parent) {
        m_shadow = m_parent->m_shadow;

        m_layer = parent->m_layer;

        m_shadow = parent->m_shadow;
        m_useLight = parent->m_useLight;
        m_useParticles = parent->m_useParticles;
        m_useDecals = parent->m_useDecals;
        m_useEmission = parent->m_useEmission;
        m_useFog = parent->m_useFog;
        m_useBloom = parent->m_useBloom;
        m_useScreenspaceEffects = parent->m_useScreenspaceEffects;

        m_forceSolid = m_parent->m_forceSolid;
        m_forceLineMode = m_parent->m_forceLineMode;

        m_allowLineMode = m_parent->m_allowLineMode;
        m_allowDrawDebug = m_parent->m_allowDrawDebug;
    }

    m_camera->setupProjection(
        m_aspectRatio,
        m_nearPlane,
        m_farPlane);

    prepareUBOs();
}

RenderContext::~RenderContext()
{
}

void RenderContext::prepareUBOs()
{
    //KI_INFO_OUT(fmt::format("ts: {}", m_data.u_time));
    auto* mainCamera = getMainCamera();
    const render::DebugContext* const dbg = m_dbg;
    auto& assets = m_assets;
    auto& selectionRegistry = *m_registry->m_selectionRegistry;

    {
        m_matricesUBO.u_view = m_camera->getView();
        m_matricesUBO.u_invView = glm::inverse(m_matricesUBO.u_view);

        m_matricesUBO.u_projection = m_camera->getProjection();
        m_matricesUBO.u_invProjection = glm::inverse(m_matricesUBO.u_projection);

        m_matricesUBO.u_projected = m_camera->getProjected();

        m_matricesUBO.u_mainProjected = m_parent ? m_parent->m_camera->getProjected() : m_camera->getProjected();

        m_matricesUBO.u_viewportMatrix = util::getViewportMatrix(m_parent ? m_parent->m_resolution : m_resolution);

        {
            // https://www.rioki.org/2013/03/07/glsl-skybox.html
            // NOTE KI remove translation from the view matrix for skybox
            glm::mat4 m = m_matricesUBO.u_view;
            m[3][0] = 0.f;
            m[3][1] = 0.f;
            m[3][2] = 0.f;

            m_matricesUBO.u_viewSkybox = glm::inverse(m) * glm::inverse(m_matricesUBO.u_projection);

            const auto& planes = m_camera->getFrustumPlanes();
            std::copy(
                std::begin(planes),
                std::end(planes),
                std::begin(m_matricesUBO.u_frustumPlanes));
        }

        if (m_parent) {
            copyShadowMatrixFrom(*m_parent);
        }
    }

    m_dataUBO = {
        m_camera->getWorldPosition(),
        //0,
        m_camera->getViewFront(),
        //0,
        m_camera->getViewUp(),
        //0,
        m_camera->getViewRight(),
        //0,
        mainCamera->getWorldPosition(),
        //0,
        mainCamera->getViewFront(),
        //0,
        mainCamera->getViewUp(),
        //0,
        mainCamera->getViewRight(),
        //0,

        dbg->m_fogColor,
        // NOTE KI keep original screen resolution across the board
        // => current buffer resolution is separately in bufferInfo UBO
        //m_parent ? m_parent->m_resolution : m_resolution,

        selectionRegistry.getSelectionMaterialIndex(),
        selectionRegistry.getTagMaterialIndex(),

        m_camera->getNearPlane(),
        m_camera->getFarPlane(),

        dbg->m_cubeMapEnabled,
        assets.skyboxEnabled,

        assets.environmentMapEnabled,

        dbg->m_shadowVisual,
        m_allowLineMode && dbg->m_forceLineMode,

        dbg->m_fogStart,
        dbg->m_fogEnd,
        dbg->m_fogDensity,

        dbg->m_effectBloomThreshold,

        dbg->m_gammaCorrect,
        dbg->m_hdrExposure,

        static_cast<float>(m_clock.ts),
        static_cast<int>(m_clock.frameCount),

        // NOTE KI u_shadowPlanes not initialized
        0, // shadowCount
    };

    if (dbg) {
        float parallaxDepth = -1.f;
        if (!dbg->m_parallaxEnabled) {
            parallaxDepth = 0;
        }
        else if (dbg->m_parallaxDebugEnabled) {
            parallaxDepth = dbg->m_parallaxDebugDepth;
        }

        m_debugUBO = {
            dbg->m_wireframeLineColor,
            dbg->m_wireframeOnly,
            dbg->m_wireframeLineWidth,

            dbg->m_entityId,
            dbg->m_animationBoneIndex,
            dbg->m_animationDebugBoneWeight,

            m_useLight && dbg->m_lightEnabled,
            m_useLight && dbg->m_normalMapEnabled,
            parallaxDepth,
            dbg->m_parallaxMethod,
        };
    }
    else {
        m_debugUBO = {
            {0.f, 0.f, 0.f},
            false,
            0.f,
            0, // entity
            0,
            false,
            true, // light
            true,
            0.f,
            0,
        };
    }

    m_clipPlanes.u_clipCount = 0;
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
    updateDebugUBO();
    updateClipPlanesUBO();
    updateLightsUBO();
}

void RenderContext::updateMatricesUBO() const
{
    validateRender("update_matrices_ubo");
    m_renderData->updateMatrices(m_matricesUBO);
}

void RenderContext::updateDataUBO() const
{
    validateRender("update_data_ubo");
    m_renderData->updateData(m_dataUBO);
}

void RenderContext::updateDebugUBO() const
{
    validateRender("update_debug_ubo");
    m_renderData->updateDebug(m_debugUBO);
}

void RenderContext::updateClipPlanesUBO() const
{
    validateRender("update_clip_planes_ubo");
    m_renderData->updateClipPlanes(m_clipPlanes);
}

void RenderContext::updateLightsUBO() const
{
    validateRender("update_lights_ubo");
    m_renderData->updateLights(m_collection);
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

void RenderContext::copyShadowMatrixFrom(const RenderContext& b)
{
    std::copy(
        std::begin(b.m_matricesUBO.u_shadow),
        std::end(b.m_matricesUBO.u_shadow),
        std::begin(m_matricesUBO.u_shadow));

    //std::copy(
    //    std::begin(b.m_matrices.u_shadowProjected),
    //    std::end(b.m_matrices.u_shadowProjected),
    //    std::begin(m_matrices.u_shadowProjected));
}

UpdateContext RenderContext::toUpdateContext() const
{
    return {
        m_clock,
        m_registry
    };
}

PrepareContext RenderContext::toPrepareContext() const
{
    return {
        m_registry,
    };
}

glm::vec3 RenderContext::getScreenDirection(
    const glm::vec2& screenPoint) const
{
    const auto startPos = unproject(screenPoint, .01f);
    const auto endPos = unproject(screenPoint, .8f);
    return glm::normalize(endPos - startPos);
}

glm::vec3 RenderContext::unproject(const glm::vec2& screenPoint, float deviceZ) const
{
    // Convert screenPoint to device coordinates (between -1 and +1)
    glm::vec3 deviceCoord = { screenPoint.x, screenPoint.y, deviceZ };
    deviceCoord.x /= m_resolution.x * 0.5f;
    deviceCoord.y /= m_resolution.y * 0.5f;
    deviceCoord.x -= 1.f;
    deviceCoord.y -= 1.f;
    deviceCoord.y *= -1;

    // Transform vector by unprojection matrix
    glm::mat4 unprojection = glm::inverse(m_camera->getProjected());

    return util::transformWithPerspDiv(deviceCoord, unprojection, 1);
}

render::Camera* const RenderContext::getMainCamera() const noexcept
{
    auto* curr = this;
    while (curr->m_parent) {
        curr = curr->m_parent;
    }
    return curr->m_camera;
}
