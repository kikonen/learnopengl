#pragma once

#include <vector>
#include <string>
#include <functional>
#include <mutex>
#include <atomic>

#include "kigl/GLSyncQueue.h"

#include "registry/MaterialRegistry.h"
#include "registry/NodeRegistry.h"
#include "registry/MeshTypeRegistry.h"
#include "registry/ModelRegistry.h"
#include "registry/EntityRegistry.h"

#include "command/CommandEngine.h"
#include "command/ScriptEngine.h"

#include "renderer/ObjectIdRenderer.h"
#include "renderer/NormalRenderer.h"

#include "ParticleSystem.h"

#include "TextureBuffer.h"
#include "WindowBuffer.h"


class Camera;
class Light;
class ParticleGenerator;
class Node;
class NodeController;
class RenderData;
class Batch;
class RenderContext;

class NodeRenderer;
//class TerrainRenderer
class ViewportRenderer;

class WaterMapRenderer;
class MirrorMapRenderer;
class CubeMapRenderer;
class ShadowMapRenderer;
class SkyboxRenderer;


class Scene final
{
public:
    Scene(
        const Assets& assets);
    ~Scene();

    void prepare(ShaderRegistry& shaders);

    void attachNodes();

    void processEvents(RenderContext& ctx);
    void update(RenderContext& ctx);
    void bind(RenderContext& ctx);
    void unbind(RenderContext& ctx);

    void draw(RenderContext& ctx);

    void drawMain(RenderContext& ctx);
    void drawRear(RenderContext& ctx);
    void drawViewports(RenderContext& ctx);

    void drawScene(RenderContext& ctx);

    Camera* getCamera() const;
    NodeController* getCameraController() const;

    void bindComponents(Node& node);
    int getObjectID(const RenderContext& ctx, double posx, double posy);

    //void bindPendingChildren();

private:
    void updateMainViewport(RenderContext& ctx);

public:
    const Assets& m_assets;

    std::shared_ptr<std::atomic<bool>> m_alive;


    std::shared_ptr<MaterialRegistry> m_materialRegistry;
    std::shared_ptr<MeshTypeRegistry> m_typeRegistry;
    std::shared_ptr<ModelRegistry> m_modelRegistry;
    std::shared_ptr<NodeRegistry> m_nodeRegistry;

    std::unique_ptr<RenderData> m_renderData;
    std::unique_ptr<EntityRegistry> m_entityRegistry;

    std::unique_ptr<CommandEngine> m_commandEngine;
    std::unique_ptr<ScriptEngine> m_scriptEngine;

    std::unique_ptr<Batch> m_batch;

protected:

private:
    bool m_prepared = false;


    std::vector<ParticleGenerator*> m_particleGenerators;

    std::unique_ptr<NodeRenderer> m_nodeRenderer{ nullptr };

    //std::unique_ptr<TerrainRenderer> terrainRenderer{ nullptr };
    std::unique_ptr<ViewportRenderer> m_viewportRenderer{ nullptr };

    std::unique_ptr<WaterMapRenderer> m_waterMapRenderer{ nullptr };
    std::unique_ptr<MirrorMapRenderer> m_mirrorMapRenderer{ nullptr };
    std::unique_ptr<CubeMapRenderer> m_cubeMapRenderer{ nullptr };
    std::unique_ptr<ShadowMapRenderer> m_shadowMapRenderer{ nullptr };

    std::unique_ptr<ObjectIdRenderer> m_objectIdRenderer{ nullptr };
    std::unique_ptr<NormalRenderer> m_normalRenderer{ nullptr };

    std::unique_ptr<ParticleSystem> m_particleSystem{ nullptr };

    std::unique_ptr<TextureBuffer> m_rearBuffer{ nullptr };
    std::shared_ptr<Viewport> m_rearViewport;

    std::unique_ptr<TextureBuffer> m_mainBuffer{ nullptr };
    std::shared_ptr<Viewport> m_mainViewport;

    std::unique_ptr<WindowBuffer> m_windowBuffer{ nullptr };

    unsigned int m_pbo = 0;
};
