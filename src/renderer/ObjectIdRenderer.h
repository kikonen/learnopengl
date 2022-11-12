#pragma once

#include "Renderer.h"

#include "scene/TextureBuffer.h"

class ObjectIdRenderer final : public Renderer
{
public:
    ObjectIdRenderer();
    virtual ~ObjectIdRenderer();

    int getObjectId(const RenderContext& ctx, double screenPosX, double screenPosY, Viewport* mainViewport);

    virtual void prepare(const Assets& assets, ShaderRegistry& shaders) override;

    virtual void update(const RenderContext& ctx, const NodeRegistry& registry) override;

    void render(
        const RenderContext& ctx,
        const NodeRegistry& registry);

private:
    void drawNodes(const RenderContext& ctx, const NodeRegistry& registry);

public:
    std::shared_ptr<Viewport> m_debugViewport;

private:
    Shader* m_idShader{ nullptr };
    Shader* m_idShaderAlpha{ nullptr };
    Shader* m_idShaderSprite{ nullptr };

    std::unique_ptr<TextureBuffer> m_idBuffer{ nullptr };
};

