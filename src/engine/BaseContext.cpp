#include "BaseContext.h"

#include "asset/Assets.h"
#include "debug/DebugContext.h"
#include "engine/Engine.h"

#include "model/Node.h"
#include "component/CameraComponent.h"

#include "kigl/GLState.h"

#include "render/RenderContext.h"
#include "render/Camera.h"

#include "scene/Scene.h"

namespace
{
    render::Camera DEFAULT_CAMERA{};
}

BaseContext::BaseContext(
    Engine& engine)
    : m_engine{ engine }
{
}

Registry* BaseContext::getRegistry() const noexcept
{
    return m_engine.getRegistry();
}

Scene* BaseContext::getScene() const noexcept
{
    return m_engine.getCurrentScene().get();
}

const Assets& BaseContext::getAssets() const noexcept
{
    return Assets::get();
}

kigl::GLState& BaseContext::getGLState() const noexcept
{
    return kigl::GLState::get();
}

const debug::DebugContext& BaseContext::getDebug() const noexcept
{
    return debug::DebugContext::get();
}

const ki::RenderClock& BaseContext::getClock() const noexcept
{
    return m_engine.getClock();
}

BaseContext::operator PrepareContext() const
{
    return toPrepareContext();
}

PrepareContext BaseContext::toPrepareContext() const
{
    return {
        m_engine
    };
}

render::RenderContext BaseContext::toRenderContext() const
{
    const auto& assets = Assets::get();
    glm::ivec2 size = m_engine.getSize();

    auto* scene = m_engine.getCurrentScene().get();

    render::Camera* camera = &DEFAULT_CAMERA;
    {
        auto* cameraNode = scene->getActiveCameraNode();
        if (cameraNode) {
            camera = &cameraNode->m_camera->getCamera();
        }
    }

    render::RenderContext ctx{
        "TOP",
        nullptr,
        m_engine.getClock(),
        getRegistry(),
        scene->getCollection(),
        m_engine.getRenderData(),
        m_engine.getBatch(),
        camera,
        assets.nearPlane,
        assets.farPlane,
        size.x,
        size.y,
        debug::DebugContext::get()
    };
    {
        // TODO KI layer needed for getObjectId()
        // => but doing this here globally is bad
        const auto* layer = LayerInfo::findLayer(LAYER_MAIN);
        //if (!layer || !layer->m_enabled) return;
        ctx.m_layer = layer->m_index;
    }
    return ctx;
}
