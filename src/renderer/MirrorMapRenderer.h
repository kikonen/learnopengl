#pragma once

#include "Renderer.h"

#include "asset/Material.h"

#include "component/Camera.h"

#include "model/Viewport.h"

class Node;
class SkyboxRenderer;
class TextureBuffer;
class Viewport;

class MirrorMapRenderer final : public Renderer
{
public:
    MirrorMapRenderer();
    ~MirrorMapRenderer();

    virtual void prepare(
        const Assets& assets,
        Registry* registry) override;

    void bindTexture(const RenderContext& ctx);

    void render(
        const RenderContext& ctx,
        SkyboxRenderer* skybox);

private:
    void drawNodes(const RenderContext& ctx, SkyboxRenderer* skybox, Node* current);
    Node* findClosest(const RenderContext& ctx);

public:
    std::shared_ptr<Viewport> m_debugViewport;

private:
    float m_nearPlane = 0.1f;
    float m_farPlane = 1000.0f;

    std::vector<Camera> m_cameras;

    std::unique_ptr<TextureBuffer> m_prev{ nullptr };
    std::unique_ptr<TextureBuffer> m_curr{ nullptr };

    Material m_tagMaterial;
};
