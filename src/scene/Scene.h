#pragma once

#include <vector>
#include <string>
#include <functional>
#include <mutex>
#include <atomic>

#include "asset/Assets.h"

#include "backend/gl/PerformanceCounters.h"


namespace event {
    class Dispatcher;
}

class Camera;
class Light;
class ParticleGenerator;

class Node;
class Viewport;

class NodeController;

class RenderData;
class Batch;
class NodeDraw;

class Registry;

class UpdateContext;
class UpdateViewContext;
class RenderContext;

class FrameBuffer;
class WindowBuffer;

class NodeRenderer;
class ViewportRenderer;

class WaterMapRenderer;
class MirrorMapRenderer;
class CubeMapRenderer;
class ShadowMapRenderer;

class ObjectIdRenderer;
class NormalRenderer;
class ParticleSystem;


class Scene final
{
public:
    Scene(
        const Assets& assets,
        std::shared_ptr<Registry> registry,
        std::shared_ptr<std::atomic<bool>> alive);
    ~Scene();

    void destroy();

    void prepareView();

    void update(const UpdateContext& ctx);
    void updateView(const UpdateViewContext& ctx);

    void handleNodeAdded(Node* node);

    void bind(const RenderContext& ctx);
    void unbind(const RenderContext& ctx);

    backend::gl::PerformanceCounters getCounters(bool clear) const;
    backend::gl::PerformanceCounters getCountersLocal(bool clear) const;

    void draw(const RenderContext& ctx);

    void drawMain(const RenderContext& ctx);
    void drawRear(const RenderContext& ctx);
    void drawViewports(const RenderContext& ctx);

    void drawScene(
        const RenderContext& ctx,
        NodeRenderer* nodeRenderer);

    Node* getActiveNode() const;
    const std::vector<NodeController*>* getActiveNodeControllers() const;

    Node* getActiveCamera() const;
    const std::vector<NodeController*>* getActiveCameraControllers() const;

    ki::node_id getObjectID(const RenderContext& ctx, float posx, float posy);

public:
    std::shared_ptr<Viewport> m_mainViewport{ nullptr };
    std::shared_ptr<Viewport> m_rearViewport{ nullptr };

public:
    const Assets& m_assets;

    std::unique_ptr<RenderData> m_renderData;

    std::shared_ptr<Registry> m_registry;

    std::unique_ptr<Batch> m_batch;
    std::unique_ptr<NodeDraw> m_nodeDraw;

protected:

private:
    bool m_loaded{ false };

    std::shared_ptr<std::atomic<bool>> m_alive;

    std::unique_ptr<NodeRenderer> m_mainRenderer{ nullptr };
    std::unique_ptr<NodeRenderer> m_rearRenderer{ nullptr };

    std::unique_ptr<ViewportRenderer> m_viewportRenderer{ nullptr };

    std::unique_ptr<WaterMapRenderer> m_waterMapRenderer{ nullptr };
    std::unique_ptr<MirrorMapRenderer> m_mirrorMapRenderer{ nullptr };
    std::unique_ptr<CubeMapRenderer> m_cubeMapRenderer{ nullptr };
    std::unique_ptr<ShadowMapRenderer> m_shadowMapRenderer{ nullptr };

    std::unique_ptr<ObjectIdRenderer> m_objectIdRenderer{ nullptr };
    std::unique_ptr<NormalRenderer> m_normalRenderer{ nullptr };

    std::unique_ptr<ParticleSystem> m_particleSystem{ nullptr };

    std::vector<Node*> m_particleGenerators;

    std::unique_ptr<WindowBuffer> m_windowBuffer{ nullptr };

    unsigned int m_pbo = 0;
};
