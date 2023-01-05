#pragma once

#include <vector>
#include <string>
#include <functional>
#include <mutex>

#include "kigl/GLSyncQueue.h"

#include "model/Node.h"
#include "component/Light.h"
#include "RenderContext.h"
#include "Batch.h"

#include "registry/MaterialRegistry.h"
#include "registry/NodeRegistry.h"
#include "registry/MeshTypeRegistry.h"
#include "registry/ModelRegistry.h"

#include "command/CommandEngine.h"
#include "command/ScriptEngine.h"

#include "renderer/NodeRenderer.h"
//#include "renderer/TerrainRenderer.h"
#include "renderer/ViewportRenderer.h"

#include "renderer/WaterMapRenderer.h"
#include "renderer/MirrorMapRenderer.h"
#include "renderer/CubeMapRenderer.h"
#include "renderer/ShadowMapRenderer.h"

#include "renderer/SkyboxRenderer.h"

#include "renderer/ObjectIdRenderer.h"
#include "renderer/NormalRenderer.h"

#include "ParticleSystem.h"

#include "TextureBuffer.h"
#include "WindowBuffer.h"

class Camera;
class NodeController;

class Scene final
{
public:
    Scene(const Assets& assets);
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

    void bindPendingChildren();

private:
    void updateMainViewport(RenderContext& ctx);
    void prepareUBOs();

public:
    const Assets& assets;

    std::unique_ptr<SkyboxRenderer> m_skyboxRenderer{ nullptr };
    UBO m_ubo;

    MaterialRegistry m_materialRegistry;
    MeshTypeRegistry m_typeRegistry;
    ModelRegistry m_modelRegistry;
    NodeRegistry m_registry;

    CommandEngine m_commandEngine;
    ScriptEngine m_scriptEngine;

    Batch m_batch;

    GLSyncQueue<TextureUBO> m_textureBuffer{ 1, TEXTURE_COUNT };
    int m_textureLevel = -1;

protected:

private:
    bool m_prepared = false;

    std::vector<ParticleGenerator*> particleGenerators;

    std::unique_ptr<NodeRenderer> m_nodeRenderer{ nullptr };

    //std::unique_ptr<TerrainRenderer> terrainRenderer{ nullptr };
    std::unique_ptr<ViewportRenderer> m_viewportRenderer{ nullptr };

    std::unique_ptr<WaterMapRenderer> m_waterMapRenderer{ nullptr };
    std::unique_ptr<MirrorMapRenderer> m_mirrorMapRenderer{ nullptr };
    std::unique_ptr<CubeMapRenderer> m_cubeMapRenderer{ nullptr };
    std::unique_ptr<ShadowMapRenderer> m_shadowMapRenderer{ nullptr };

    std::unique_ptr<ObjectIdRenderer> m_objectIdRenderer{ nullptr };
    std::unique_ptr<NormalRenderer> m_normalRenderer{ nullptr };

    std::unique_ptr<ParticleSystem> particleSystem{ nullptr };

    std::unique_ptr<TextureBuffer> m_rearBuffer{ nullptr };
    std::shared_ptr<Viewport> m_rearViewport;

    std::unique_ptr<TextureBuffer> m_mainBuffer{ nullptr };
    std::shared_ptr<Viewport> m_mainViewport;

    std::unique_ptr<WindowBuffer> m_windowBuffer{ nullptr };

    unsigned int m_pbo = 0;
};
