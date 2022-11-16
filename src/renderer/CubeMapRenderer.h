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
    void bindTexture(const RenderContext& ctx);

    void render(
        const RenderContext& ctx,
        const NodeRegistry& registry,
        SkyboxRenderer* skybox);

private:
    void clearCubeMap(
        const RenderContext& ctx,
        DynamicCubeMap& cube,
        const glm::vec4& color,
        bool debug);

    void drawNodes(
        const RenderContext& ctx,
        const NodeRegistry& registry,
        SkyboxRenderer* skybox,
        const Node* centerNode);

    Node* findCenter(
        const RenderContext& ctx,
        const NodeRegistry& registry);

public:

private:
    //glm::vec3 center{ 0, 0, 0 };

    float m_nearPlane = 0.1f;
    float m_farPlane = 1000.0f;

    bool m_cleared{ false };

    std::unique_ptr<DynamicCubeMap> m_prev;
    std::unique_ptr<DynamicCubeMap> m_curr;

    std::vector<Camera> m_cameras;
};
