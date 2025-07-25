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

#include "render/Camera.h"
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
    const render::DebugContext& dbg)
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
    const render::DebugContext& dbg = m_dbg;
    auto& assets = m_assets;

    {
        auto& matricesUBO = m_cameraUBO;
        matricesUBO.u_view = m_camera->getView();
        matricesUBO.u_invView = glm::inverse(matricesUBO.u_view);

        matricesUBO.u_projection = m_camera->getProjection();
        matricesUBO.u_invProjection = glm::inverse(matricesUBO.u_projection);

        matricesUBO.u_projected = m_camera->getProjected();

        matricesUBO.u_mainProjected = m_parent ? m_parent->m_camera->getProjected() : m_camera->getProjected();

        matricesUBO.u_viewportMatrix = util::getViewportMatrix(m_parent ? m_parent->m_resolution : m_resolution);

        {
            // https://www.rioki.org/2013/03/07/glsl-skybox.html
            // NOTE KI remove translation from the view matrix for skybox
            glm::mat4 m = matricesUBO.u_view;
            m[3][0] = 0.f;
            m[3][1] = 0.f;
            m[3][2] = 0.f;

            matricesUBO.u_viewSkybox = glm::inverse(m) * glm::inverse(matricesUBO.u_projection);

            const auto& planes = m_camera->getFrustumPlanes();
            std::copy(
                std::begin(planes),
                std::end(planes),
                std::begin(matricesUBO.u_frustumPlanes));
        }

        if (m_parent) {
            copyShadowMatrixFrom(*m_parent);
        }
    }

    {
        auto& cameraUBO = m_cameraUBO;
        cameraUBO.u_cameraPos = m_camera->getWorldPosition();
        cameraUBO.u_cameraFront = m_camera->getViewFront();
        cameraUBO.u_cameraUp = m_camera->getViewUp();
        cameraUBO.u_cameraRight = m_camera->getViewRight();
        cameraUBO.u_mainCameraPos = mainCamera->getWorldPosition();
        cameraUBO.u_mainCameraFront = mainCamera->getViewFront();
        cameraUBO.u_mainCameraUp = mainCamera->getViewUp();
        cameraUBO.u_mainCameraRight = mainCamera->getViewRight();

        cameraUBO.u_nearPlane = m_camera->getNearPlane();
        cameraUBO.u_farPlane = m_camera->getFarPlane();

        cameraUBO.u_cameraSsaoEnabled = m_useSsao;
    }

    // NOTE KI keep clipping
    if (m_parent) {
        m_clipPlanesUBO = m_parent->m_clipPlanesUBO;
    }
    else {
        m_clipPlanesUBO.u_clipCount = 0;
    }
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
    updateCameraUBO();
    //updateClipPlanesUBO();
}

void RenderContext::updateCameraUBO() const
{
    validateRender("update_camera_ubo");

    {
        auto& cameraUBO = m_cameraUBO;
        cameraUBO.u_cameraSsaoEnabled = m_useSsao;
    }

    m_renderData->updateCamera(m_cameraUBO);
}

void RenderContext::updateClipPlanesUBO() const
{
    validateRender("update_clip_planes_ubo");
    m_renderData->updateClipPlanes(m_clipPlanesUBO);
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
        std::begin(b.m_cameraUBO.u_shadow),
        std::end(b.m_cameraUBO.u_shadow),
        std::begin(m_cameraUBO.u_shadow));

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
