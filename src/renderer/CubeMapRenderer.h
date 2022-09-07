#pragma once

#include "Renderer.h"

#include "asset/DynamicCubeMap.h"
#include "scene/TextureBuffer.h"

class SkyboxRenderer;

class CubeMapRenderer final : public Renderer
{
public:
    CubeMapRenderer(const Assets& assets);
    virtual ~CubeMapRenderer();

    void prepare(ShaderRegistry& shaders) override;
    void bind(const RenderContext& ctx);
    void bindTexture(const RenderContext& ctx);
    void render(const RenderContext& ctx, NodeRegistry& registry, SkyboxRenderer* skybox);

private:
    void drawNodes(const RenderContext& ctx, NodeRegistry& registry);
    Node* findCenter(const RenderContext& ctx, NodeRegistry& registry);

public:
    glm::vec3 center{ 0, 0, 0 };

private:
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;

    std::unique_ptr<DynamicCubeMap> cubeMap;
};
