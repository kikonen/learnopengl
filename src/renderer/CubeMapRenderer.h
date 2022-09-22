#pragma once

#include "Renderer.h"

#include "asset/DynamicCubeMap.h"
#include "scene/TextureBuffer.h"

class SkyboxRenderer;

class CubeMapRenderer final : public Renderer
{
public:
    CubeMapRenderer();
    virtual ~CubeMapRenderer();

    void prepare(const Assets& assets, ShaderRegistry& shaders) override;
    void bind(const RenderContext& ctx);
    void bindTexture(const RenderContext& ctx);

    void render(
        const RenderContext& ctx,
        const NodeRegistry& registry,
        SkyboxRenderer* skybox);

private:
    void drawNodes(
        const RenderContext& ctx,
        const NodeRegistry& registry,
        SkyboxRenderer* skybox,
        const Node* centerNode);

    Node* findCenter(
        const RenderContext& ctx,
        const NodeRegistry& registry);

public:
    glm::vec3 center{ 0, 0, 0 };

private:
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;

    std::unique_ptr<DynamicCubeMap> cubeMap;
};
