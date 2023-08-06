#pragma once

#include "Renderer.h"

#include "model/Viewport.h"

class FrameBuffer;

class ObjectIdRenderer final : public Renderer
{
public:
    ObjectIdRenderer() {}
    virtual ~ObjectIdRenderer() {};

    int getObjectId(
        const RenderContext& ctx,
        double screenPosX,
        double screenPosY,
        Viewport* mainViewport);

    virtual void prepare(
        const Assets& assets,
        Registry* registry) override;

    void updateView(
        const RenderContext& ctx);

    void render(
        const RenderContext& ctx);

private:
    void drawNodes(const RenderContext& ctx);

public:
    std::shared_ptr<Viewport> m_debugViewport;

private:
    Program* m_idProgram{ nullptr };
    //Program* m_idProgramPointSprite{ nullptr };

    std::unique_ptr<FrameBuffer> m_idBuffer{ nullptr };
};
