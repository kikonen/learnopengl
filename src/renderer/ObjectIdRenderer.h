#pragma once

#include "Renderer.h"

#include "scene/TextureBuffer.h"

#include "model/Viewport.h"

class ObjectIdRenderer final : public Renderer
{
public:
    ObjectIdRenderer();
    virtual ~ObjectIdRenderer();

    int getObjectId(
        const RenderContext& ctx,
        double screenPosX,
        double screenPosY,
        Viewport* mainViewport);

    virtual void prepare(
        const Assets& assets,
        Registry* registry) override;

    virtual void update(const RenderContext& ctx) override;

    void render(
        const RenderContext& ctx);

private:
    void drawNodes(const RenderContext& ctx);

public:
    std::shared_ptr<Viewport> m_debugViewport;

private:
    Program* m_idProgram{ nullptr };
    Program* m_idProgramSprite{ nullptr };

    std::unique_ptr<TextureBuffer> m_idBuffer{ nullptr };
};
