#pragma once

#include <vector>
#include <string>
#include <functional>
#include <mutex>
#include <atomic>

#include "backend/gl/PerformanceCounters.h"

#include "kigl/GLSyncQueue.h"

#include "render/CubeMap.h"

#include "renderer/NodeRenderer.h"
#include "renderer/ViewportRenderer.h"

#include "renderer/WaterMapRenderer.h"
#include "renderer/MirrorMapRenderer.h"
#include "renderer/CubeMapRenderer.h"
#include "renderer/ShadowMapRenderer.h"

#include "renderer/ObjectIdRenderer.h"
#include "renderer/NormalRenderer.h"

#include "ParticleSystem.h"


namespace event {
    class Dispatcher;
}

class Camera;
class Light;
class ParticleGenerator;
class Node;
class NodeController;
class RenderData;
class Batch;
class NodeDraw;

class Registry;

class CommandEngine;
class ScriptEngine;

class UpdateContext;
class RenderContext;
class FrameBuffer;
class WindowBuffer;

class NodeRenderer;
class ViewportRenderer;

class WaterMapRenderer;
class MirrorMapRenderer;
class CubeMapRenderer;
class ShadowMapRenderer;


class Scene final
{
public:
    Scene(
        const Assets& assets,
        std::shared_ptr<Registry> registry);
    ~Scene();

    void prepare();

    void processEvents(const UpdateContext& ctx);
    void update(const UpdateContext& ctx);
    void updateView(const RenderContext& ctx);

    void bind(const RenderContext& ctx);
    void unbind(const RenderContext& ctx);

    backend::gl::PerformanceCounters getCounters(bool clear);
    backend::gl::PerformanceCounters getCountersLocal(bool clear);

    void draw(const RenderContext& ctx);

    void drawMain(const RenderContext& ctx);
    void drawRear(const RenderContext& ctx);
    void drawViewports(const RenderContext& ctx);

    void drawScene(
        const RenderContext& ctx,
        FrameBuffer* targetBuffer);

    Node* getActiveCamera() const;
    NodeController* getActiveCameraController() const;

    void bindComponents(Node* node);
    int getObjectID(const RenderContext& ctx, double posx, double posy);

    //void bindPendingChildren();

private:
    void updateMainViewport(const RenderContext& ctx);

public:
    const Assets& m_assets;

    std::shared_ptr<std::atomic<bool>> m_alive;

    std::unique_ptr<RenderData> m_renderData;

    std::shared_ptr<Registry> m_registry;

    std::unique_ptr<CommandEngine> m_commandEngine;
    std::unique_ptr<ScriptEngine> m_scriptEngine;

    std::unique_ptr<Batch> m_batch;
    std::unique_ptr<NodeDraw> m_nodeDraw;

protected:

private:
    NodeRenderer m_nodeRenderer;
    ViewportRenderer m_viewportRenderer;

    WaterMapRenderer m_waterMapRenderer;
    MirrorMapRenderer m_mirrorMapRenderer;
    CubeMapRenderer m_cubeMapRenderer;
    ShadowMapRenderer m_shadowMapRenderer;

    ObjectIdRenderer m_objectIdRenderer;
    NormalRenderer m_normalRenderer;

    std::unique_ptr<ParticleSystem> m_particleSystem{ nullptr };

    std::vector<Node*> m_particleGenerators;

    std::unique_ptr<FrameBuffer> m_rearBuffer{ nullptr };
    std::shared_ptr<Viewport> m_rearViewport;

    std::unique_ptr<FrameBuffer> m_mainBuffer{ nullptr };
    std::shared_ptr<Viewport> m_mainViewport;

    std::unique_ptr<WindowBuffer> m_windowBuffer{ nullptr };

    unsigned int m_pbo = 0;

    CubeMap m_skyboxMap{ false };
};
