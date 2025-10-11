#pragma once

#include <vector>
#include <string>
#include <functional>
#include <atomic>

#include "shader/ShadowUBO.h"

#include "event/Listen.h"

namespace render {
    class NodeCollection;
}

namespace model
{
    class Node;
    class Viewport;
}

namespace render
{
    class RenderContext;
}

class Light;

class NodeController;

class Engine;

struct UpdateContext;
struct UpdateViewContext;

class LayerRenderer;
class ViewportRenderer;

class WaterMapRenderer;
class MirrorMapRenderer;
class CubeMapRenderer;
class ShadowMapRenderer;

class ObjectIdRenderer;

namespace editor {
    class EditorFrame;
    class CameraTool;
    class NodeEditTool;
    class ViewportTool;
}

class Scene final
{
    friend class editor::EditorFrame;
    friend class editor::CameraTool;
    friend class editor::NodeEditTool;
    friend class editor::ViewportTool;

public:
    Scene(
        Engine& engine);
    ~Scene();

    void clear();
    void destroy();

    void prepareRT();

    void updateRT(const UpdateContext& ctx);
    void postRT(const UpdateContext& ctx);
    void updateViewRT(const UpdateViewContext& ctx);

    void bind(const render::RenderContext& ctx);
    void unbind(const render::RenderContext& ctx);

    void render(const render::RenderContext& ctx);

    model::Node* getActiveNode() const;
    const std::vector<std::unique_ptr<NodeController>>* getActiveNodeControllers() const;

    model::Node* getActiveCameraNode() const;
    const std::vector<std::unique_ptr<NodeController>>* getActiveCameraControllers() const;

    ki::node_id getObjectID(const render::RenderContext& ctx, float posx, float posy);

    render::NodeCollection* getCollection()
    {
        return m_collection.get();
    }

    const std::string& getName() const noexcept
    {
        return m_name;
    }

    void setName(const std::string& name)
    {
        m_name = name;
    }

    const std::string& getFilePath() const noexcept
    {
        return m_filePath;
    }

    void setFilePath(const std::string& filePath)
    {
        m_filePath = filePath;
    }

private:
    void renderUi(const render::RenderContext& ctx);
    void renderPlayer(const render::RenderContext& ctx);
    void renderMain(const render::RenderContext& ctx);
    void renderRear(const render::RenderContext& ctx);
    void renderViewports(const render::RenderContext& ctx);

    void renderScene(
        const render::RenderContext& ctx,
        LayerRenderer* layerRenderer);

    void handleNodeAdded(model::Node* node);
    void handleNodeRemoved(model::Node* node);

    void updateShadowUBO() const;
    void updateLightsUBO() const;

private:
    Engine& m_engine;
    std::shared_ptr<std::atomic_bool> m_alive;

    std::string m_name;
    std::string m_filePath;
    bool m_loaded{ false };

    event::Listen m_listen_scene_loaded;
    event::Listen m_listen_node_added;
    event::Listen m_listen_node_removed;
    event::Listen m_listen_camera_activate;
    event::Listen m_listen_camera_activate_next;

    std::unique_ptr<render::NodeCollection> m_collection;

    ShadowUBO m_shadowUBO;

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

    std::shared_ptr<model::Viewport> m_uiViewport{ nullptr };
    std::shared_ptr<model::Viewport> m_playerViewport{ nullptr };
    std::shared_ptr<model::Viewport> m_mainViewport{ nullptr };
    std::shared_ptr<model::Viewport> m_rearViewport{ nullptr };
};
