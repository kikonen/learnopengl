#pragma once

#include <vector>
#include <string>
#include <functional>
#include <mutex>
#include <atomic>

#include "pool/NodeHandle.h"

#include "backend/gl/PerformanceCounters.h"


namespace event {
    class Dispatcher;
}

namespace render {
    class NodeDraw;
    class Batch;
    class RenderData;
    class FrameBuffer;
    class WindowBuffer;
}

class Camera;
class Light;
class ParticleGenerator;

class Node;
class Viewport;

class NodeController;

class Registry;

struct UpdateContext;
struct UpdateViewContext;
class RenderContext;

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
        std::shared_ptr<Registry> registry,
        std::shared_ptr<std::atomic<bool>> alive);
    ~Scene();

    void destroy();

    void prepareRT();

    void updateRT(const UpdateContext& ctx);
    void postRT(const UpdateContext& ctx);
    void updateViewRT(const UpdateViewContext& ctx);

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

    Node* getActiveCameraNode() const;
    const std::vector<NodeController*>* getActiveCameraControllers() const;

    ki::node_id getObjectID(const RenderContext& ctx, float posx, float posy);

public:
    std::shared_ptr<Viewport> m_mainViewport{ nullptr };
    std::shared_ptr<Viewport> m_rearViewport{ nullptr };

public:
    std::unique_ptr<render::RenderData> m_renderData;

    std::shared_ptr<Registry> m_registry;

    std::unique_ptr<render::Batch> m_batch;
    std::unique_ptr<render::NodeDraw> m_nodeDraw;

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

    std::vector<pool::NodeHandle> m_particleGenerators;

    std::unique_ptr<render::WindowBuffer> m_windowBuffer{ nullptr };

    unsigned int m_pbo = 0;
};
