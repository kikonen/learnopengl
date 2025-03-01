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
    //class NodeDraw;
    class Batch;
    class RenderData;
    class WindowBuffer;
    class NodeCollection;
}

class Light;

class Node;
class Viewport;

class NodeController;

class Registry;

struct UpdateContext;
struct UpdateViewContext;
class RenderContext;

class LayerRenderer;
class ViewportRenderer;

class WaterMapRenderer;
class MirrorMapRenderer;
class CubeMapRenderer;
class ShadowMapRenderer;

class ObjectIdRenderer;
class NormalRenderer;
class PhysicsRenderer;
class VolumeRenderer;
class EnvironmentProbeRenderer;

namespace editor {
    class EditorFrame;
}

class Scene final
{
    friend class editor::EditorFrame;
public:
    Scene(
        std::shared_ptr<Registry> registry,
        std::shared_ptr<std::atomic<bool>> alive);
    ~Scene();

    void clear();
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

    void drawUi(const RenderContext& ctx);
    void drawPlayer(const RenderContext& ctx);
    void drawMain(const RenderContext& ctx);
    void drawRear(const RenderContext& ctx);
    void drawViewports(const RenderContext& ctx);

    void drawScene(
        const RenderContext& ctx,
        LayerRenderer* layerRenderer);

    Node* getActiveNode() const;
    const std::vector<NodeController*>* getActiveNodeControllers() const;

    Node* getActiveCameraNode() const;
    const std::vector<NodeController*>* getActiveCameraControllers() const;

    ki::node_id getObjectID(const RenderContext& ctx, float posx, float posy);

    render::NodeCollection* getCollection()
    {
        return m_collection.get();
    }

public:
    std::shared_ptr<Viewport> m_uiViewport{ nullptr };
    std::shared_ptr<Viewport> m_playerViewport{ nullptr };
    std::shared_ptr<Viewport> m_mainViewport{ nullptr };
    std::shared_ptr<Viewport> m_rearViewport{ nullptr };

public:
    std::unique_ptr<render::RenderData> m_renderData;

    std::shared_ptr<Registry> m_registry;

    std::unique_ptr<render::Batch> m_batch;
    //std::unique_ptr<render::NodeDraw> m_nodeDraw;

protected:

private:
    bool m_loaded{ false };

    std::unique_ptr<render::NodeCollection> m_collection;

    std::shared_ptr<std::atomic<bool>> m_alive;

    std::unique_ptr<LayerRenderer> m_uiRenderer{ nullptr };
    std::unique_ptr<LayerRenderer> m_playerRenderer{ nullptr };
    std::unique_ptr<LayerRenderer> m_mainRenderer{ nullptr };
    std::unique_ptr<LayerRenderer> m_rearRenderer{ nullptr };

    std::unique_ptr<ViewportRenderer> m_viewportRenderer{ nullptr };

    std::unique_ptr<WaterMapRenderer> m_waterMapRenderer{ nullptr };
    std::unique_ptr<MirrorMapRenderer> m_mirrorMapRenderer{ nullptr };
    std::unique_ptr<CubeMapRenderer> m_cubeMapRenderer{ nullptr };
    std::unique_ptr<ShadowMapRenderer> m_shadowMapRenderer{ nullptr };

    std::unique_ptr<ObjectIdRenderer> m_objectIdRenderer{ nullptr };
    std::unique_ptr<NormalRenderer> m_normalRenderer{ nullptr };

    std::unique_ptr<PhysicsRenderer> m_physicsRenderer{ nullptr };
    std::unique_ptr<VolumeRenderer> m_volumeRenderer{ nullptr };
    std::unique_ptr<EnvironmentProbeRenderer> m_environmentProbeRenderer{ nullptr };

    std::unique_ptr<render::WindowBuffer> m_windowBuffer{ nullptr };
};
