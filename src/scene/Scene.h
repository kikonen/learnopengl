#pragma once

#include <vector>
#include <string>
#include <functional>
#include <mutex>

#include "model/Node.h"
#include "component/Light.h"
#include "NodeRegistry.h"
#include "RenderContext.h"

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

    Camera* getCamera();

    void bindComponents(Node& node);
    int getObjectID(const RenderContext& ctx, double posx, double posy);

    void bindPendingChildren();

private:
    void updateMainViewport(RenderContext& ctx);
    void prepareUBOs();

public:
    const Assets& assets;

    std::unique_ptr<SkyboxRenderer> skyboxRenderer{ nullptr };
    UBO ubo;

    NodeRegistry registry;
    CommandEngine commandEngine;
    ScriptEngine scriptEngine;

protected:

private:
    bool m_prepared = false;

    std::vector<ParticleGenerator*> particleGenerators;

    std::unique_ptr<NodeRenderer> nodeRenderer{ nullptr };

    //std::unique_ptr<TerrainRenderer> terrainRenderer{ nullptr };
    std::unique_ptr<ViewportRenderer> viewportRenderer{ nullptr };

    std::unique_ptr<WaterMapRenderer> waterMapRenderer{ nullptr };
    std::unique_ptr<MirrorMapRenderer> mirrorMapRenderer{ nullptr };
    std::unique_ptr<CubeMapRenderer> cubeMapRenderer{ nullptr };
    std::unique_ptr<ShadowMapRenderer> shadowMapRenderer{ nullptr };

    std::unique_ptr<ObjectIdRenderer> objectIdRenderer{ nullptr };
    std::unique_ptr<NormalRenderer> normalRenderer{ nullptr };

    std::unique_ptr<ParticleSystem> particleSystem{ nullptr };

    std::unique_ptr<TextureBuffer> m_readBuffer{ nullptr };
    std::shared_ptr<Viewport> m_rearViewport;

    std::unique_ptr<TextureBuffer> mainBuffer{ nullptr };
    std::shared_ptr<Viewport> mainViewport;

    unsigned int pbo = 0;
};
