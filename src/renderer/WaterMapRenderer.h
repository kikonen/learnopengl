#pragma once

#include "Renderer.h"
#include "scene/TextureBuffer.h"

#include "model/Viewport.h"

class SkyboxRenderer;

class WaterMapRenderer final : public Renderer
{
public:
    WaterMapRenderer();
    virtual ~WaterMapRenderer();

    virtual void prepare(
        const Assets& assets,
        ShaderRegistry& shaders,
        MaterialRegistry& materialRegistry) override;

    void bindTexture(const RenderContext& ctx);

    void render(
        const RenderContext& ctx,
        SkyboxRenderer* skybox);

private:
    void drawNodes(
        const RenderContext& ctx,
        SkyboxRenderer* skybox,
        Node* current,
        bool reflect);

    Node* findClosest(const RenderContext& ctx);

public:
    std::shared_ptr<Viewport> m_reflectionDebugViewport;
    std::shared_ptr<Viewport> m_refractionDebugViewport;

private:
    float m_nearPlane{ 0.1f };
    float m_farPlane{ 1000.0f };

    std::vector<Camera> m_cameras;

    std::unique_ptr<TextureBuffer> m_reflectionBuffer{ nullptr };
    std::unique_ptr<TextureBuffer> m_refractionBuffer{ nullptr };

    unsigned int m_noiseTextureID{ 0 };
};
